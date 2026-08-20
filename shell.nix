with import <nixpkgs> {};
mkShell {
  NIX_ENFORCE_NO_NATIVE=0;

  name = "env";
  nativeBuildInputs = [ cmake clang gcc ccache ];
  buildInputs = [ fftw fftwFloat llvmPackages.openmp ];
}
