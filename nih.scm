(define-module (nih)
  #:use-module (guix build-system cmake)
  #:use-module (guix git-download)
  #:use-module (guix packages)
  #:use-module ((guix licenses) #:prefix license:)
  #:use-module (gnu packages check))

(define-public nih
  (let* ((commit "55c4b73264c5e1fa4a5f33cbb70b8a1658193b36")
	 (revision "0")
	 (version (git-version "0.0.0" revision commit)))
    (package
      (name "nih")
      (version version)
      (home-page "https://github.com/trivialfis/nih")
      (source (origin
		(method git-fetch)
		(uri (git-reference
                      (url home-page)
                      (commit commit)
		      (recursive? #t)))
		(sha256
		 (base32
		  "0mbsybvkzm62w54zyw61kp5fdbmwsyh9n0f38y04kyq2jmsxq3nd"))
		(file-name (git-file-name name version))))
      (arguments
       `(#:configure-flags
	 '("-DCXX_STANDARD=ON")))
      (native-inputs
       `(("googletest" ,googletest)))
      (build-system cmake-build-system)
      (synopsis "")
      (description "")
      (license license:lgpl3+))))

(define-public nih-local
  (package
    (name "nih-local")
      (version "0.0.0")
      (home-page "https://github.com/trivialfis/nih")
      (source (dirname (current-filename)))
      (native-inputs
       `(("googletest" ,googletest)))
      (build-system cmake-build-system)
      (synopsis "")
      (description "")
      (license license:lgpl3+)))