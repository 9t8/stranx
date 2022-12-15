; defines
(define a 3)
(define b a)
1
2
a
b

; lambdas
(lambda (x) x)
((lambda (x) x) 4)
(define a (lambda (x) x))
(a 1)
(a 2)
((lambda () 2))
(define (b) 2)
b
(b)

; true/false
(define if (lambda (c t f) (c t f)))
(define t (lambda (l r) l))
(define f (lambda (l r) r))
(if t 1 0)
(if f 1 0)

; swapped names to check for mangling
(define (if c f t) (c f t))
(define (t l r) l)
(define (f l r) r)
(if t 1 0)
(if f 1 0)

; multi-statement procedures
(lambda () 0 1)
((lambda () 0 1))
(define (f a b) a b)
(f 0 1)
