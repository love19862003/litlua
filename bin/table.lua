local t1 = { 
	a = 1,
	b = 2, 
	c = "....",
	f = function( str) print(str)  return 1 end,
}

function getTable() 
	return t1, 1, 2
end

function print_table( t, indent)
 indent = indent.." "
 print(indent.."[")
 for k, v in pairs( t ) do 
	if type(v) == "table" then 
		print( indent .." "..k .. " = ")
		print_table(v, indent)
	else 
		if type(v) == "function" then 
			print(  indent .. " " ..k .. " = function")
		else
			print(  indent .. " " ..k .. " = " .. v)
		end
	end
 end
  print(indent.."]")
 
end