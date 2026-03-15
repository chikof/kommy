{
  inputs = {
    flakelight-rust.url = "github:accelbread/flakelight-rust";
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = {flakelight-rust, ...}:
    flakelight-rust ./. {
      devShell = {
        packages = pkgs: [
          pkgs.hidapi
          pkgs.libusb1
          pkgs.udev
          pkgs.llvmPackages.libclang
          pkgs.llvmPackages.clang
          pkgs.pkg-config
        ];

        env = pkgs: {
          LIBCLANG_PATH = "${pkgs.llvmPackages.libclang.lib}/lib";
          BINDGEN_EXTRA_CLANG_ARGS = ''
            -I${pkgs.llvmPackages.libclang.lib}/lib/clang/${pkgs.llvmPackages.llvm.version}/include \
            -I${pkgs.glibc.dev}/include
          '';
          PKG_CONFIG_PATH = "${pkgs.hidapi}/lib/pkgconfig";
        };
      };
    };
}
