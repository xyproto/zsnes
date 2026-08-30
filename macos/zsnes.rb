class Zsnes < Formula
  desc "Super Nintendo emulator"
  homepage "https://github.com/xyproto/zsnes"
  url "https://github.com/xyproto/zsnes/archive/refs/tags/2.2.3.tar.gz"
  sha256 "d6770c3b4c9f1b594ab303e3e2266da0bb8ef9f1b9030ae7b47135e21bd071cf"
  license "GPL-2.0-only"
  head "https://github.com/xyproto/zsnes.git", branch: "main"

  depends_on "pkgconf" => :build
  depends_on "python@3.13" => :build
  depends_on "libpng"
  depends_on "sdl3"

  uses_from_macos "zlib"

  def install
    system "make", "PREFIX=#{prefix}"
    system "make", "install", "PREFIX=#{prefix}"
  end

  test do
    # The self-test checks the allocations, the pointers handed between units
    # and the config written on exit, and needs no window or ROM.
    ENV["ZSNES_SELFTEST"] = "1"
    ENV["SDL_VIDEODRIVER"] = "dummy"
    ENV["SDL_AUDIODRIVER"] = "dummy"
    system bin/"zsnes", "-v", "0"
  end
end
