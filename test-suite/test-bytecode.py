import dis

code = compile("x = 1", "<stdin>", "exec")
dis.dis(code)