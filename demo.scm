;; Arithmatic
(+ (* 4 5) (/ 10 2))

;; Assignment
(define x 6)
(+ x x)

;; Lists


;; Lambda functions

;; Higher order functions
(define f
  (lambda (x)
    (lambda (y)
      (+ x y))))
(f 4)
