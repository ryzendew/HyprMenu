# Maintainer: Matt <matt@hyprmenu.org>

pkgname=hyprmenu
pkgver=0.1.0
pkgrel=1
pkgdesc="A modern application launcher for Hyprland"
arch=('x86_64')
url="https://github.com/hyprland-community/hyprmenu"
license=('MIT')
depends=(
  'gtk4'
  'gtk4-layer-shell'
  'hicolor-icon-theme'
  'glib2'
)
makedepends=(
  'meson'
  'ninja'
  'gcc'
  'pkgconf'
)
optdepends=(
  'ttf-font-awesome: for icon support'
)
backup=(
  "etc/hyprmenu/hyprmenu.conf"
)
source=("$pkgname-$pkgver.tar.gz::$url/archive/v$pkgver.tar.gz")
sha256sums=('SKIP')

build() {
  cd "$pkgname-$pkgver"
  meson setup build \
    --prefix=/usr \
    --buildtype=release \
    --wrap-mode=nofallback
  meson compile -C build
}

package() {
  cd "$pkgname-$pkgver"
  meson install -C build --destdir "$pkgdir"
} 