### HW4

In most programming languages, when a program enters an **infinite loop**, it typically doesn't return any value, as the program continues to execute indefinitely. However, Python has a unique feature called `yield`, which allows a function to return values even within an infinite loop.

The interesting aspect of `yield` is that it seems to implement a **state machine** within the program, allowing the program to pause and resume execution while retaining the current state.

### Exercise:

1. **Understand and practice the following Python code:**
   ```python
   def numbers(init=0, step=1):
       n = init
       while True:
           n += step
           yield n

   g = numbers()

   print(next(g)) 
   print(next(g))  
   print(next(g))  
   ```
   - Practice and understand this code, and think about how `yield` works to maintain and resume the state of the function.

2. **Think about how to implement similar functionality in C:**
   - Explore different methods. If you can allocate stack space and save registers for the state machine in C, that would be ideal.
