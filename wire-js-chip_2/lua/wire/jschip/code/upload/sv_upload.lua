util.AddNetworkString("JSChip_RequestCode")

local scriptChunks = {}

net.Receive("JSChip_RequestCode", function(len, ply)
    local totalChunks = net.ReadUInt(16)
    local chunkIndex = math.Clamp(net.ReadUInt(16), 1, totalChunks)
    local chunkData = net.ReadString()
    local chip = ply.JSChipTarget

    if not IsValid(chip) then return end

    scriptChunks[ply] = scriptChunks[ply] or {chunks = {}, total = 0}

    scriptChunks[ply].chunks[chunkIndex] = chunkData
    scriptChunks[ply].total = totalChunks

    local storedChunks = scriptChunks[ply].chunks
    if table.Count(storedChunks) == totalChunks then
        local completeScript = table.concat(storedChunks)
        scriptChunks[ply] = nil

        local err, bytecode, length = JS_Compile(chip.ctx, #completeScript, completeScript)

        if err ~= 0 then
          chip:Error(bytecode)
        else
          chip.Run = true
          chip:SetColor(Color(255, 255, 255))

          local err, result = JS_Eval(chip.ctx, length, bytecode)
          if err ~= 0 then
            chip:Error(result)
          end

          chip.script = completeScript
        end
    end
end)