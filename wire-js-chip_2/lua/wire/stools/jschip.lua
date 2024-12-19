WireToolSetup.setCategory( "Chips, Gates", "Advanced" )
WireToolSetup.open( "jschip", "JS Chip", "gmod_wire_jschip", nil, "CPUs" )

if CLIENT then
  language.Add("Tool.wire_jschip.name", "JS Chip Tool (Wire)")
  language.Add("Tool.wire_jschip.desc", "Spawns a javascript chip")
  language.Add("ToolWirejschip_Model",  "Model:" )
  TOOL.Information = {
    { name = "left", text = "Paste code" },
    { name = "right", text = "Open editor" },
  }
end
WireToolSetup.BaseLang()
WireToolSetup.SetupMax( 7 )

TOOL.ClientConVar = {
  model             = "models/cheeze/wires/cpu.mdl",
  filename          = "",
  extensions        = ""
}

if CLIENT then  ------------------------------------------------------------------------------
  -- Make sure firing animation is displayed clientside
  ------------------------------------------------------------------------------
  function TOOL:LeftClick()  return true end
  function TOOL:Reload()     return false end
  function TOOL:RightClick() return false end
end


if SERVER then
  util.AddNetworkString("JSChip_RequestCode")
  util.AddNetworkString("JSChip_OpenEditor")
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
  
  function TOOL:RightClick(trace)
    net.Start("JSChip_OpenEditor") net.Send(self:GetOwner())
    return true
  end
  
  function TOOL:LeftClick_Update(trace, ent)
    local ply = self:GetOwner()
    local chip = ent or trace.Entity
    ply.JSChipTarget = chip

    if chip.ctx then
      JS_FreeContext(chip.ctx)
    end

    local err, ctx = JS_CreateContext(chip, ply)
    if err ~= 0 then
      chip:Error(result)
      chip:Remove()
      return
    end

    chip.ctx = ctx

    net.Start("JSChip_RequestCode") net.Send(ply)
  end

  function TOOL:MakeEnt(ply, model, Ang, trace)
    local ent = WireLib.MakeWireEnt(ply, {Class = self.WireClass, Pos=trace.HitPos, Angle=Ang, Model=model})

    self:LeftClick_Update(trace, ent)
    return ent
  end

end

function JSChip_SetupEditor(editor,title,location,name)
    editor:Setup(title,location,name)
end

function JSChip_OpenEditor()
  if not JS_Editor then
    JS_Editor = vgui.Create("Expression2EditorFrame")
    JSChip_SetupEditor(JS_Editor,"JS Editor", "jschip", "JS")
  end
  JS_Editor:Open()
end
net.Receive("JSChip_OpenEditor", JSChip_OpenEditor)

function TOOL.BuildCPanel(panel)
  local Button = vgui.Create("DButton" , panel)
  panel:AddPanel(Button)

  ----------------------------------------------------------------------------
  local currentDirectory
  local FileBrowser = vgui.Create("wire_expression2_browser" , panel)
  panel:AddPanel(FileBrowser)
  FileBrowser:Setup("JSChip")
  FileBrowser:SetSize(235,400)
  function FileBrowser:OnFileOpen(filepath, newtab)
    JSChip_OpenEditor()
    JS_Editor:Open(filepath, nil, newtab)
  end


  ----------------------------------------------------------------------------
  local New = vgui.Create("DButton" , panel)
  panel:AddPanel(New)
  New:SetText("New file")
  New.DoClick = function(button)
    JSChip_OpenEditor()
    JS_Editor:AutoSave()
    JS_Editor:NewScript(false)
  end
  ----------------------------------------------------------------------------
  local OpenEditor = vgui.Create("DButton", panel)
  panel:AddPanel(OpenEditor)
  OpenEditor:SetText("Open Editor")
  OpenEditor.DoClick = JSChip_OpenEditor

end
