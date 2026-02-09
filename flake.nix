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
              (arduino-nix.latestVersion EspSoftwareSerial)
              (arduino-nix.latestVersion LiquidCrystal)
            ];

            packages = with pkgs.arduinoPackages; [
              platforms.esp32.esp32."3.3.2"
            ];
          };
          fqbn_xiao = "esp32:esp32:XIAO_ESP32C6";
          fqbn_wroom = "esp32:esp32:esp32da";
          mkFirmware = type: fqbn: pkgs.stdenv.mkDerivation rec {
            name = "bsafe";
            src = ./.;
            buildInputs = [
              arduino-cli
              pkgs.python3
            ];
            buildPhase = ''
              arduino-cli compile --fqbn ${fqbn} --export-binaries --build-property compiler.cpp.extra_flags=-D${type} ./${name}
            '';
            installPhase = ''
              cp -r bsafe/build/* $out/
            '';
          };
          mkApp = pkg: port: fqbn: {
            type = "app";
            program = "${pkgs.writeShellScript "upload.sh" ''
              ${arduino-cli}/bin/arduino-cli upload --port ${port} --fqbn ${fqbn} --build-path ${pkg}
            ''}";
          };
          sensor = mkFirmware "SENSOR" fqbn_wroom;
          receiver = mkFirmware "RECEIVER" fqbn_wroom;
        in {
          formatter = pkgs.nixpkgs-fmt;

          packages.sensor = sensor;
          packages.receiver = receiver;
          apps.sensor = mkApp sensor "/dev/ttyUSB0" fqbn_wroom;
          apps.receiver = mkApp receiver "/dev/ttyUSB0" fqbn_wroom;

          devShells.default = pkgs.mkShell {
            buildInputs = [
              arduino-cli
              pkgs.python3
            ];
          };
        }
      ));
}
