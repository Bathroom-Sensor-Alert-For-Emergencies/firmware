{
  description = "Senior design project";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    arduino-nix.url = "github:bouk/arduino-nix";
    arduino-index = {
      url = "github:bouk/arduino-indexes";
      flake = false;
    };
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
    arduino-nix,
    arduino-index,
    ...
  } @ attrs:
    let
      overlays = [
        (arduino-nix.overlay)
        (arduino-nix.mkArduinoPackageOverlay (arduino-index + "/index/package_index.json"))
        (arduino-nix.mkArduinoPackageOverlay (arduino-index + "/index/package_esp32_index.json"))
        (arduino-nix.mkArduinoPackageOverlay (arduino-index + "/index/package_seeeduino_boards_index.json"))
        (arduino-nix.mkArduinoLibraryOverlay (arduino-index + "/index/library_index.json"))
      ];
    in
      (flake-utils.lib.eachDefaultSystem (system:
        let
          pkgs = (import nixpkgs) { inherit system overlays; };
          arduino-cli = pkgs.wrapArduinoCLI {
            libraries = with pkgs.arduinoLibraries; [
              (arduino-nix.latestVersion RadioLib)
            ];

            packages = with pkgs.arduinoPackages; [
              platforms.esp32.esp32."3.3.2"
            ];
          };
          fqbn = "esp32:esp32:XIAO_ESP32C6";
          port = "/dev/ttyACM0";
        in {
          formatter = pkgs.nixpkgs-fmt;

          packages.default = pkgs.stdenv.mkDerivation rec {
            name = "bsafe";
            src = ./.;
            buildInputs = [
              arduino-cli
              pkgs.python3
            ];
            buildPhase = ''
              arduino-cli compile --fqbn ${fqbn} --export-binaries ./${name}
            '';
            installPhase = ''
              cp -r bsafe/build/* $out/
            '';
          };

          apps.default = {
            type = "app";
            program = "${pkgs.writeShellScript "upload.sh" ''
              ${arduino-cli}/bin/arduino-cli upload --port ${port} --fqbn ${fqbn} \
                --build-path ${self.packages.${system}.default}
            ''}";
          };

          devShells.default = pkgs.mkShell {
            buildInputs = [
              arduino-cli
              pkgs.python3
              pkgs.picocom
            ];
          };
        }
      ));
}
