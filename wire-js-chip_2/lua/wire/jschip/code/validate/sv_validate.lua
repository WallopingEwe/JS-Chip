util.AddNetworkString("JSChip_ValidateCode")

local scriptChunks = {}

net.Receive("JSChip_ValidateCode", function(len, ply)
    local totalChunks = net.ReadUInt(16)
    local chunkIndex = math.Clamp(net.ReadUInt(16), 1, totalChunks)
    local chunkData = net.ReadString()

    scriptChunks[ply] = scriptChunks[ply] or {chunks = {}, total = 0}

    scriptChunks[ply].chunks[chunkIndex] = chunkData
    scriptChunks[ply].total = totalChunks

    local storedChunks = scriptChunks[ply].chunks
    if table.Count(storedChunks) == totalChunks then
        local completeScript = table.concat(storedChunks)
        scriptChunks[ply] = nil

        local err, ctx = JS_CreateContext(ply)
        if err ~= 0 then
          net.Start("JSChip_ValidateCode")
          net.WriteBool(true, 16)
          net.WriteString("failed to validate the script")
          net.Send(ply)
        else
          local err, bytecode, length = JS_Compile(ctx, #completeScript, completeScript)
          net.Start("JSChip_ValidateCode")
          net.WriteBool(err ~= 0, 16)
          net.WriteString(bytecode)
          net.Send(ply)
          JS_FreeContext(ctx)
        end
    end
end)