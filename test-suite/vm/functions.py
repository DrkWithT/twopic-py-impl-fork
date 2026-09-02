import dis

code = """
x = 4

def add(a, b):
    total = a + b
    return total

x = add(2, 3)
"""

compiled = compile(code, "<string>", "exec")
dis.disassemble(compiled)