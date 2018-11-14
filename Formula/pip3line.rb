class Pip3line < Formula
  desc "Swiss Army knife for raw bytes manipulation & interception"
  homepage "https://metrodango.github.io/pip3line/index.html"
  url "https://github.com/metrodango/pip3line/archive/v3.5.4.tar.gz"
  sha256 "c3bcd1028db0f74303b64d8b16913e09ad1ebb5bc63abf2dfa4d0b1ab463a88f"
  depends_on "cmake" => :build
  depends_on "openssl"
  depends_on "python"
  depends_on "qscintilla2"
  depends_on "qt"

  def install
    system "cmake", ".", *std_cmake_args, "-DALL=ON", "-DWITH_PYTHON27=OFF", "-DWITH_SCINTILLA=ON", "-DWITH_DISTORM_LINK_STATICALLY=ON"
    system "make", "install"
  end
  test do
    system "echo 'testing the beast' | pip3linecmd -t Base64 | pip3linecmd -t Base64 -o"
  end
end
