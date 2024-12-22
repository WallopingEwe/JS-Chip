net.Receive("JSChip_ChatPrint", function()
    chat.AddText(net.ReadString())
end)