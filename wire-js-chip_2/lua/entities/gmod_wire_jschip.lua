AddCSLuaFile()
DEFINE_BASECLASS( "base_wire_entity" )
ENT.PrintName       = "Wire JS"
ENT.Author          = "Huh"
ENT.WireDebugName	= "JSCHIP"

if CLIENT then return end

require("javascript");

function ENT:Initialize()
	self:PhysicsInit(SOLID_VPHYSICS)
	self:SetMoveType(MOVETYPE_VPHYSICS)
	self:SetSolid(SOLID_VPHYSICS)
	self:SetOverlayText("JS Chip")

	self.Inputs = Wire_CreateInputs(self, {})
	self.Outputs = Wire_CreateOutputs(self, {})
	
	local err, ctx = JS_CreateContext()
	if err ~= 0 then
		self:Error(err)
		self:Remove()
	end
	self.ctx = ctx
	self.Run = false
	self.Code = ""
end

function ENT:Think()
	if self.Run then
		local err, result = JS_Eval(self.ctx, self.CodeLength, self.Code)
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
end

function ENT:ApplyDupeInfo(ply, ent, info, GetEntByID)
	BaseClass.ApplyDupeInfo(self, ply, ent, info, GetEntByID)
end

function ENT:TriggerInput(iname, value)
end

duplicator.RegisterEntityClass("gmod_wire_jschip", WireLib.MakeWireEnt, "Data")
