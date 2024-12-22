util.AddNetworkString("JSChip_ChatPrint")

JSChatPrint = function(ply, ...)
	if CurTime() < (ply.JSCanPrint or 0) then return end
	
	net.Start("JSChip_ChatPrint")
	net.WriteString(string.sub(table.concat({ ... }, "\t"), 1, 64000))
	net.Send(ply)

	ply.JSCanPrint = CurTime() + 0.2
end