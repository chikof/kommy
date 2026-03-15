use std::sync::Arc;
use std::sync::atomic::{AtomicBool, Ordering};
use std::thread;
use std::time::{Duration, Instant};
use wooting_rgb::{RgbKeyboard, is_wooting_keyboard_connected};

// Wooting 80HE layout: 6 rows × 17 columns
const ROWS: usize = 6;
const COLS: usize = 17;

const FLOW_DIRECTION: f32 = -1.0; // -1.0 = right, 1.0 = left

const LUT_SIZE: usize = 1024;
fn build_lut() -> [(u8, u8, u8); LUT_SIZE] {
    let mut lut = [(0u8, 0u8, 0u8); LUT_SIZE];
    for (i, val) in lut.iter_mut().enumerate().take(LUT_SIZE) {
        *val = blossom_gradient(i as f32 / LUT_SIZE as f32);
    }
    lut
}

fn blossom_gradient(t: f32) -> (u8, u8, u8) {
    let colors: &[(u8, u8, u8)] = &[
        (180, 20, 70),   // deep rose
        (255, 80, 130),  // hot pink
        (255, 160, 190), // light pink
        (255, 225, 235), // blush white
        (220, 130, 180), // lavender pink
    ];

    let n = colors.len() as f32;
    let scaled = t.rem_euclid(1.0) * n;
    let idx = scaled.floor() as usize % colors.len();
    let next = (idx + 1) % colors.len(); // wraps 4 → 0 seamlessly
    let frac = scaled.fract();

    lerp_color(colors[idx], colors[next], frac)
}

fn lerp_color(a: (u8, u8, u8), b: (u8, u8, u8), t: f32) -> (u8, u8, u8) {
    let lerp = |a: u8, b: u8| (a as f32 + (b as f32 - a as f32) * t) as u8;
    (lerp(a.0, b.0), lerp(a.1, b.1), lerp(a.2, b.2))
}

fn main() {
    let mut kb = RgbKeyboard;

    if !is_wooting_keyboard_connected() {
        eprintln!("Wooting keyboard not found!");
        return;
    }

    let running = Arc::new(AtomicBool::new(true));
    let r = running.clone();

    ctrlc::set_handler(move || {
        r.store(false, Ordering::Relaxed);
    })
    .expect("Error setting Ctrl+C handler");

    println!("Cherry blossom wave starting... Ctrl+C to exit.");

    let lut = build_lut();
    let mut time: f32 = 0.0;
    let frame_target = Duration::from_millis(10);
    let dir_x = 0.8;
    let dir_y = 0.3;

    while running.load(Ordering::Relaxed) {
        let frame_start = Instant::now();

        for row in 0..ROWS {
            for col in 0..COLS {
                let gradient_pos = ((col as f32 * dir_x + row as f32 * dir_y) / COLS as f32
                    + FLOW_DIRECTION * time)
                    .rem_euclid(1.0);

                let lut_idx = (gradient_pos * LUT_SIZE as f32) as usize % LUT_SIZE;
                let (r, g, b) = lut[lut_idx];
                kb.array_set_single((row as u8, col as u8), r, g, b);
            }
        }

        kb.array_update();
        time = (time + 0.005).rem_euclid(1.0);

        if let Some(remaining) = frame_target.checked_sub(frame_start.elapsed()) {
            thread::sleep(remaining);
        }
    }

    println!("\nRestoring keyboard lighting...");
    kb.reset_all();
}
