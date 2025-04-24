{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = with pkgs; [
    clang
    pkg-config
    xorg.libxcb
    xorg.xcbproto
    xorg.xcbutil
  ];
}
