;; Custom settings
;; Set the standards for Makefiles
(defun rc-makefile-settings ()
  (setq-default tab-width 8)
  (setq-default tab-stop-list '(8 16 24 32 40))
)


(setq default-tab-width 8)
(add-hook 'makefile-mode-hook 'rc-makefile-settings)
(setq c-default-style "linux")
