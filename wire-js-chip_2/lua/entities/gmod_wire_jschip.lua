AddCSLuaFile()
DEFINE_BASECLASS( "base_wire_entity" )
ENT.PrintName       = "Wire JS"
ENT.Author          = "Huh"
ENT.WireDebugName	= "JSCHIP"

if CLIENT then return end

function ENT:Initialize()
	self:PhysicsInit(SOLID_VPHYSICS)
	self:SetMoveType(MOVETYPE_VPHYSICS)
	self:SetSolid(SOLID_VPHYSICS)
	self:SetOverlayText("JS Chip")

	self.Inputs = Wire_CreateInputs(self, {})
	self.Outputs = Wire_CreateOutputs(self, {})
	
	self.Run = false
end

function ENT:Think()
	if self.Run then
		local err, result = JS_EmitEvent(self.ctx, "tick")
		if err ~= 0 then
			self:Error(result)
		end
	end

	self:NextThink( CurTime() )
	return true
end

function ENT:OnRemove()
	JS_FreeContext(self.ctx)
end

function ENT:Error(Str)
	self.Run = false
	self:SetColor(Color(255, 0, 0))
	WireLib.ClientError(Str, WireLib.GetOwner(self))
end

function ENT:BuildDupeInfo()
	local info = BaseClass.BuildDupeInfo(self) or {}
	
	info.script = self.script

	return info
end

function ENT:ApplyDupeInfo(ply, ent, info, GetEntByID)
	BaseClass.ApplyDupeInfo(self, ply, ent, info, GetEntByID)
    
	self.script = info.script

    local err, ctx = JS_CreateContext(self, ply)
    if err ~= 0 then
        self:Error(result)
        self:Remove()
        return
    end

	local err, bytecode, length = JS_Compile(ctx, #self.script, self.script)
    if err ~= 0 then
    	self:Error(bytecode)
		return
	end

    local err, result = JS_Eval(ctx, length, bytecode)
    if err ~= 0 then
        self:Error(result)
		return
    end

	self:SetColor(Color(255, 255, 255))
	self.Run = true
    self.ctx = ctx
end

function ENT:SetInputs(inputTable)
	local names = {}
	local types = {}

	for key, value in pairs(inputTable) do
		table.insert(names, key)
		table.insert(types, string.upper(value))
	end

	self.Inputs = WireLib.AdjustSpecialInputs(self, names, types)
end

function ENT:SetOutputs(outputTable)
	local names = {}
	local types = {}

	for key, value in pairs(outputTable) do
		table.insert(names, key)
		table.insert(types, string.upper(value))
	end

	self.Outputs = WireLib.AdjustSpecialOutputs(self, names, types)
end

function ENT:TriggerOutput(iname, value)
	WireLib.TriggerOutput(self, iname, value)
end

function ENT:OutputType(iname)
	return (self.Outputs[iname] and self.Outputs[iname].Type) or "none"
end

function ENT:TriggerInput(iname, value)
	if self.Run then
		local err, result = JS_EmitEvent(self.ctx, "input", iname, value)
		if err ~= 0 then
			self:Error(result)
		end
	end
end

duplicator.RegisterEntityClass("gmod_wire_jschip", WireLib.MakeWireEnt, "Data")
