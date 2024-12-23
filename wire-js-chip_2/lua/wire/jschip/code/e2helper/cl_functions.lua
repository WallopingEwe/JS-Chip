
JSChip_Functions = {
    Events = {
        On = {takes = "String, Function", returns = "", description = "Sets up an event listener for a string.\nEvents.on(\"tick\", () => {})\nEvents.on(\"input\", (inputName, intputValue) => {})", cost = 1}
    },
    Inputs = {
        Change = {takes = "Object", returns = "", description = "Changes the chips inputs. {input: \"number\"}, input2: \"entity\"}, input3: \"vector\"} etc", cost = 1}
    },
    Outputs = {
        Change = {takes = "Object", returns = "", description = "Changes the chips outputs. {input: \"number\"}, input2: \"entity\"}, input3: \"vector\"} etc", cost = 1},
        Trigger = {takes = "String, *", returns = "", description = "Sets one of the chips outputs.", cost = 1}
    },
    Vector = {
        Add = {takes = "Vector, Vector or Number", returns = "Vector", description = "Adds 2 vectors together or adds a number to a vector.", cost = 1},
        Sub = {takes = "Vector, Vector or Number", returns = "Vector", description = "Subtracts 2 vectors together or subtracts a number from a vector.", cost = 1},
        Mul = {takes = "Vector, Vector or Number", returns = "Vector", description = "Multiplies 2 vectors together or multiplies a number to a vector.", cost = 1},
        Div = {takes = "Vector, Vector or Number", returns = "Vector", description = "Divides 2 vectors together or divides a vector by a number.", cost = 1},
        Mod = {takes = "Vector, Vector or Number", returns = "Vector", description = "Modulos a vector by a vector or a number.", cost = 1},
        Pow = {takes = "Vector, Vector or Number", returns = "Vector", description = "Modulos a vector by a vector or a number.", cost = 1},
        Distance = {takes = "Vector, Vector", returns = "Number", description = "Gets the distance between two vectors.", cost = 1},
        Distance2 = {takes = "Vector, Vector", returns = "Number", description = "Gets the squared distance between two vectors.", cost = 1},
        Magnitude = {takes = "Vector", returns = "Number", description = "Gets the magnitude of a vector.", cost = 1},
        Normalize = {takes = "Vector", returns = "Vector", description = "Normalizes a vector.", cost = 1}
    },
    Entity = {
        Owner = {takes = "", returns = "Number", description = "Gets the chip owner's entity ID.", cost = 1},
        Chip = {takes = "", returns = "Number", description = "Gets the chip's entity ID.", cost = 1},
        GetName = {takes = "Number", returns = "String", description = "Gets an entity's name.", cost = 1},
        GetClass = {takes = "Number", returns = "String", description = "Gets an entity's class.", cost = 1},
        ApplyForce = {takes = "Number, Vector", returns = "", description = "Applies a force to an entity's mass center.", cost = 1},
        ApplyForceOffset = {takes = "Number, Vector, Vector", returns = "", description = "Applies a force to an entity at an offset position.", cost = 1},
        IsValid = {takes = "Number", returns = "Boolean", description = "Checks if an entity is valid.", cost = 1},
        IsValidPhysics = {takes = "Number", returns = "Boolean", description = "Checks if an entity's physics object is valid.", cost = 1},
        GetPos = {takes = "Number", returns = "Vector", description = "Gets the position of an entity.", cost = 1},
        GetMassCenter = {takes = "Number", returns = "Vector", description = "Gets the center of mass of an entity's physics object.", cost = 1},
        ToWorld = {takes = "Number, Vector", returns = "Vector", description = "Converts a local position to world coordinates.", cost = 1},
        ToLocal = {takes = "Number, Vector", returns = "Vector", description = "Converts a world position to local coordinates relative to the entity.", cost = 1},
        IsWorld = {takes = "Number", returns = "Boolean", description = "Checks if an entity is the world entity.", cost = 1},
        GetColor = {takes = "Number", returns = "Color", description = "Gets the color of an entity (as an object with r, g, b, a).", cost = 1},
        GetAngles = {takes = "Number", returns = "Angle", description = "Gets the angles (rotation) of an entity.", cost = 1},
        IsPlayer = {takes = "Number", returns = "Boolean", description = "Checks if an entity is a player.", cost = 1},
        IsNPC = {takes = "Number", returns = "Boolean", description = "Checks if an entity is an NPC.", cost = 1},
        Crouching = {takes = "Number", returns = "Boolean", description = "Checks if a player entity is crouching.", cost = 1},
        EyeAngles = {takes = "Number", returns = "Angle", description = "Gets the eye angles (view direction) of a player entity.", cost = 1},
        SteamID = {takes = "Number", returns = "String", description = "Gets the SteamID of a player entity.", cost = 1},
        Team = {takes = "Number", returns = "Number", description = "Gets the team ID of a player entity.", cost = 1},
        Frags = {takes = "Number", returns = "Number", description = "Gets the number of kills (frags) a player entity has.", cost = 1},
        SteamID64 = {takes = "Number", returns = "String", description = "Gets the 64-bit SteamID of a player entity.", cost = 1},
        AccountID = {takes = "Number", returns = "Number", description = "Gets the Account ID of a player entity.", cost = 1},
        Use = {takes = "Number", returns = "", description = "Simulates the use action on an entity.", cost = 1},
        SetColor = {takes = "Number, Color", returns = "", description = "Sets the color of an entity (as an object with r, g, b, a).", cost = 1},
        SetAngles = {takes = "Number, Angle", returns = "", description = "Sets the angles (rotation) of an entity.", cost = 1},
        SetPos = {takes = "Number, Vector", returns = "", description = "Sets the position of an entity.", cost = 1},
        GetMass = {takes = "Number", returns = "Number", description = "Gets the mass of an entity's physics object.", cost = 1}
    },
    console = {
        log = {takes = "...", returns = "", description = "Prints to chat.", cost = 1}
    }
}