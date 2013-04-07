;;; Arithmatic
(+ (* 4 5) (/ 10 2))

;;; Assignment
(define x 6)
(+ x x)
;; 12

;;; Lists
(define x (cons 1 (cons '(2 4) (cons 3 nil))))
x
;; (1 (2 4) 3)
(car (cdr x))
;; (2 4)

;;; Lambda functions
(define y (lambda (x) (* x x)))
(y (y 2))
;; 16

;;; Higher order functions
(define f
  (lambda (x)
    (lambda (y)
      (+ x y))))
(f 4)
;; <procedure>
((f 3) 2)
;; 5
