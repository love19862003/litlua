
function fun1()
	print("fun1 called")
	return 1, 2, "string", 3.1, nil, {a=1, b= 2, c=3, 4,5, func = function() print("this is lua table function call") return 1, 2, 3,4 end}
end

function fun2(arg1, arg2, arg3, arg4)
	print("fun2 called")
end

function fun3( arg1)
	print("fun3 called")
end