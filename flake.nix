{
  description = "HyprMenu - A GTK4 application menu for Hyprland";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        packages.default = pkgs.stdenv.mkDerivation {
          pname = "hyprmenu";
          version = "0.1.0";
          src = builtins.path {
            path = ./.;
            name = "hyprmenu-source";
            filter = path: type:
              type != "directory" || !(builtins.elem (builtins.baseNameOf path) ["build" "result"]);
          };

          nativeBuildInputs = [
            pkgs.meson
            pkgs.ninja
            pkgs.pkg-config
            pkgs.wrapGAppsHook4
          ];

          buildInputs = [
            pkgs.gtk4
            pkgs.gtk4-layer-shell
            pkgs.glib
            pkgs.gobject-introspection
          ];

          dontWrapGApps = true;

          preFixup = ''
            gappsWrapperArgs+=(
              --prefix XDG_DATA_DIRS : "${pkgs.gtk4}/share/gsettings-schemas/${pkgs.gtk4.name}"
            )
          '';

          mesonFlags = [
            "--prefix=${placeholder "out"}"
            "--buildtype=release"
          ];
        };

        devShells.default = pkgs.mkShell {
          packages = with pkgs; [
            meson
            ninja
            pkg-config
            gtk4
            gtk4-layer-shell
            glib
            gobject-introspection
            wrapGAppsHook4
          ];
          
          shellHook = ''
            export XDG_DATA_DIRS=${pkgs.gtk4}/share/gsettings-schemas/${pkgs.gtk4.name}:$XDG_DATA_DIRS
          '';
        };
        
        apps.default = {
          type = "app";
          program = "${self.packages.${system}.default}/bin/hyprmenu";
        };
      });
}