def numbers(init=0, step=1):
    n = init
    while True:
        n += step
        yield n

g = numbers()

print(g)

print(g.__next__()) 
print(g.__next__())  
print(g.__next__())  