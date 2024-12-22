function JSChip_UploadCode(script, validate)
    local chunkSize = 64000
    local chunks = {}
  
    for i = 1, #script, chunkSize do
        chunks[#chunks + 1] = script:sub(i, i + chunkSize - 1)
    end
  
    for i, chunk in ipairs(chunks) do
        if validate then
          net.Start("JSChip_ValidateCode")
        else
          net.Start("JSChip_RequestCode")
        end
        net.WriteUInt(#chunks, 16)
        net.WriteUInt(i, 16)
        net.WriteString(chunk)
        net.SendToServer()
    end
  end
  
  net.Receive("JSChip_ValidateCode", function(len, ply)
    local err = net.ReadBool()
    local result = net.ReadString()
    if err then
      JS_Editor.C.Val:Update({{message = result, line = nil, char = nil}}, nil, result, Color(128, 20, 50))
    else
      JS_Editor.C.Val:Update(nil, nil, "   Validation successful   ", Color(50, 128, 20))
    end
  end)
  
  net.Receive("JSChip_RequestCode", function(len, ply)
    if JS_Editor then
      JSChip_UploadCode(JS_Editor:GetCode(), false)
    end
  end)
  
  function JSChip_Validate(editor,source,fileName)
    JSChip_UploadCode(source, true)
  end