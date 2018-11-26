printStr("printStr1", "printStr2", "printStr3")
printStr(1,2,3)
printStr(nil, 1, "printStr")

local a = 100 
local b = add(a)
assert(b == a + 1)
printStr(b, " = ", a .." + 1")
assert( b == test(a, b))
print(func(1, 100))
