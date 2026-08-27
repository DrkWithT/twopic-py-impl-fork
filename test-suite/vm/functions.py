import dis

code = """
def add(a, b):
    total = a + b
    return total

def double(n):
    twice = add(n, n)
    return twice

x = add(2, 3)
y = double(21)
"""

compiled = compile(code, "<string>", "exec")
dis.disassemble(compiled)