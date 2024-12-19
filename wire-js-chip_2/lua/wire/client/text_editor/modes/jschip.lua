local math_floor = math.floor
local string_gmatch = string.gmatch
local string_gsub = string.gsub
local draw_WordBox = draw.WordBox

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

local EDITOR = {
  UseValidator = true,
  Validator = JSChip_Validate,
  E2HelperCategory = "JS", -- As an override, makes GPU and SPU use the ZCPU helper too
}

hook.Add( "Initialize", "JSChip_Setup_E2Helper", function()
  E2Helper.Modes.JS = nil
  local helperModule = E2Helper:RegisterMode("JS")
  local helperDesc = helperModule.Descriptions
      local helperInst = helperModule.Items
      helperModule.ModeSetup = function(panel)
        panel.FunctionColumn:SetName("Function")
        panel.FunctionColumn:SetWidth(126)
        panel.FromColumn:SetName("From") -- This can be the extension name now
        panel.FromColumn:SetWidth(80)
        panel.TakesColumn:SetName("Takes")
        panel.TakesColumn:SetWidth(60)
        panel.ReturnsColumn:SetName("Returns")
        panel.ReturnsColumn:SetWidth(60)
        panel.CostColumn:SetName("Cost")
        panel.CostColumn:SetWidth(40)
      end

      table.insert(helperInst, { [1] = "console.log", [2] = "console", [3] = "...", [4] = "undefined", [5] = "1" })
      helperDesc["console.log"] = "Print to the console."
end )

-- CPU hint box
local oldpos, haschecked = {0,0}, false
function EDITOR:Think()
  local caret = self:CursorToCaret()
  local startpos, word = self:getWordStart( caret, true )

  if word and word ~= "" then
    if not haschecked then
      oldpos = {startpos[1],startpos[2]}
      haschecked = true
      timer.Simple(0.3,function()
        if not self then return end
        if not self.CursorToCaret then return end
        local caret = self:CursorToCaret()
        local startpos, word = self:getWordStart( caret, true )
        if startpos[1] == oldpos[1] and startpos[2] == oldpos[2] then
          self.CurrentVarValue = { startpos, word }
        end
      end)
    elseif (oldpos[1] ~= startpos[1] or oldpos[2] ~= startpos[2]) and haschecked then
      haschecked = false
      self.CurrentVarValue = nil
      oldpos = {0,0}
    end
  else
    self.CurrentVarValue = nil
    haschecked = false
    oldpos = {0,0}
  end
end

local colors = {
  ["normal"]   = { Color(126, 204, 236), false},
  ["bracket"]  = { Color(241, 215,  16), false},
  ["decs"]     = { Color(220, 220, 170), false},
  ["operator"] = { Color(212, 212, 201), false},
  ["comment"]  = { Color( 97,  151, 74), false},
  ["number"]   = { Color(176, 196, 153), false},
  ["string"]   = { Color(190, 141, 111), false},
  ["keyword"]  = { Color( 61, 135, 200), false},
  ["loop"]     = { Color(206, 112, 178), false},
  ["class"]    = { Color( 78, 188, 125), false},
  ["error"]    = { Color(240,  96,  96), false},
}

-- Build lookup table for keywords
local keywordsList = {
  "VAR", "CONST", "LET", "FUNCTION", "TRUE", "FALSE", "UNDEFINED", "TYPEOF", "INSTANCEOF", "NEW", "NULL", "ASYNC", "DELETE", "CONSTRUCTOR", "THIS", "CLASS", "VOID", "SUPER", "EXTENDS"
}

local keywordsTable = {}

for k,v in pairs(keywordsList) do
  keywordsTable[v] = true
end

-- Not all of this is used because operators are handled one by one, but it's here for reference
local operatorTable = {
  ["+"] = true,
  ["-"] = true,
  ["*"] = true,
  ["/"] = true,
  ["%"] = true,
  ["**"] = true,

  -- Assignment Operators
  ["="] = true,
  ["+="] = true,
  ["-="] = true,
  ["*="] = true,
  ["/="] = true,
  ["%="] = true,
  ["**="] = true,
  ["&="] = true,
  ["|="] = true,
  ["^="] = true,
  ["<<="] = true,
  [">>="] = true,
  [">>>="] = true,

  -- Comparison Operators
  ["=="] = true,
  ["==="] = true,
  ["!="] = true,
  ["!=="] = true,
  ["<"] = true,
  [">"] = true,
  ["<="] = true,
  [">="] = true,

  -- Logical Operators
  ["&&"] = true,
  ["||"] = true,
  ["!"] = true,

  -- Bitwise Operators
  ["&"] = true,
  ["|"] = true,
  ["^"] = true,
  ["~"] = true,
  ["<<"] = true,
  [">>"] = true,
  [">>>"] = true,

  -- Other Operators
  ["."] = true,
  [","] = true,
  [";"] = true,
  ["?:"] = true,
  ["=>"] = true,
  ["..."] = true,
  [":"] = true,

  -- Optional Chaining
  ["?."] = true
}

local bracketTable = {
  ["{"] = true,
  ["}"] = true,
  ["["] = true,
  ["]"] = true,
  ["("] = true,
  [")"] = true
}

local loopTable = {
  ["DO"] = true,
  ["WHILE"] = true,
  ["FOR"] = true,
  ["IN"] = true,
  ["AWAIT"] = true,
  ["IF"] = true,
  ["ELSE"] = true,
  ["SWITCH"] = true,
  ["CASE"] = true,
  ["DEFAULT"] = true,
  ["BREAK"] = true,
  ["RETURN"] = true,
  ["CONTINUE"] = true,
  ["TRY"] = true,
  ["CATCH"] = true,
  ["THROW"] = true
}

local stdObjectTable = {
  ["Math"] = true,
  ["Object"] = true,
  ["Number"] = true,
  ["String"] = true,
  ["Array"] = true,
  ["Date"] = true,
  ["globalThis"] = true,
  ["Map"] = true,
  ["Set"] = true,
  ["WeakSet "] = true,
  ["WeakMap"] = true,
  ["JSON"] = true,
  ["Promise"] = true,
  ["Symbol"] = true,
  ["Proxy"] = true,
  ["Atomics"] = true,
  ["BigInt"] = true,
  ["WeakRef"] = true,
  ["SharedArrayBuffer"] = true,
  ["Error"] = true,
  ["TypeError"] = true,
  ["RangeError"] = true,
  ["SyntaxError"] = true,
  ["AggregateError"] = true,
  ["EvalError"] = true,
  ["ReferenceError"] = true,
  ["Events"] = true,
  ["Inputs"] = true,
  ["Outputs"] = true,
  ["Entity"] = true
}

function EDITOR:CommentSelection(removecomment)
  local comment_char = "//"
  if removecomment then
    -- shift-TAB with a selection --
    local tmp = string_gsub("\n"..self:GetSelection(), "\n"..comment_char, "\n")

    -- makes sure that the first line is outdented
    self:SetSelection(tmp:sub(2))
  else
    -- plain TAB with a selection --
    self:SetSelection(comment_char .. self:GetSelection():gsub("\n", "\n"..comment_char))
  end
end

function EDITOR:BlockCommentSelection(removecomment)
  local sel_start, sel_caret = self:MakeSelection( self:Selection() )
  local str = self:GetSelection()
  if removecomment then
    if str:find( "^/%*" ) and str:find( "%*/$" ) then
      self:SetSelection( str:gsub( "^/%*(.+)%*/$", "%1" ) )

      sel_caret[2] = sel_caret[2] - 2
    end
  else
    self:SetSelection( "/*" .. str .. "*/" )

    if sel_caret[1] == sel_start[1] then
      sel_caret[2] = sel_caret[2] + 4
    else
      sel_caret[2] = sel_caret[2] + 2
    end
  end
  return { sel_start, sel_caret }
end

function EDITOR:ShowContextHelp(word)
  E2Helper.Show()
  E2Helper:SetMode("JS")
  E2Helper.Show(word)
end

function EDITOR:ResetTokenizer(row)
  if row == self.Scroll[1] then
    -- As above, but for HL-ZASM: Check whether the line self.Scroll[1] starts within a block comment.
    self.blockcomment = nil

    for k=1, self.Scroll[1]-1 do
      local row = self.Rows[k]

      for match in string_gmatch(row, "[/*][/*]") do
        if match == "//" then
          -- single line comment start; skip remainder of line
          break
        elseif match == "/*" then
          self.blockcomment = true
        elseif match == "*/" then
          self.blockcomment = nil
        end
      end
    end
  end
end

function EDITOR:SyntaxColorLine(row)
  local cols = {}
  self:ResetTokenizer(row)
  self:NextCharacter()

  if self.blockcomment then
    if self:NextPattern(".-%*/") then
      self.blockcomment = nil
    else
      self:NextPattern(".*") 
    end

    cols[#cols + 1] = {self.tokendata, colors["comment"]}
  end

  local prev_sstr = ""
  local curOperator = ""

  while self.character do
    local tokenname = ""
    self.tokendata = ""

    self:NextPattern(" *") -- Skip leading spaces
    if not self.character then break end

    -- Match keywords and numbers
    if self:NextPattern("^[a-zA-Z0-9_@]+") then
        local sstr_lcase = self.tokendata:Trim()
        local sstr = string.upper(sstr_lcase)

        if stdObjectTable[sstr_lcase] then
          tokenname = "class"
        elseif loopTable[sstr] then
          tokenname = "loop"
        elseif keywordsTable[sstr] then
            tokenname = "keyword"
        elseif tonumber(self.tokendata) then
            tokenname = "number"
        else
          if prev_sstr == "CLASS"  then -- make classes green
            tokenname = "class"
          elseif self.character == "("  then
            tokenname = "decs"
          else
            tokenname = "normal"
          end
        end

        prev_sstr = sstr

    -- Match strings
    elseif self.character == "'" or self.character == "\"" or self.character == "`" then
        tokenname = "string"
        local delimiter = self.character
        self:NextCharacter()
        while self.character ~= delimiter do
            if not self.character then
                tokenname = "error"
                break
            end
            if self.character == "\\" then self:NextCharacter() end
            self:NextCharacter()
        end
        self:NextCharacter()

    -- Match single-line comments
    elseif self:NextPattern("^//.*$") then
        tokenname = "comment"

    -- Match multi-line comments
    elseif self:NextPattern("^/%*") then
        tokenname = "comment"
        self.blockcomment = true
        if self:NextPattern(".-%*/") then
            self.blockcomment = nil
        else
            self:NextPattern(".*")
        end
        self:NextCharacter()

    -- Match individual operators
    else
      if operatorTable[self.character] then
        tokenname = "operator"
      elseif bracketTable[self.character] then
        tokenname = "bracket"
      else
        tokenname = "normal"
      end

      local prevOperator = self.character
      self:NextCharacter()
      
      if (prevOperator == "=" and self.character == ">") or (curOperator == "=" and prevOperator == ">") then // for =>
        tokenname = "keyword"
      end
      
      curOperator = prevOperator
    end

    local color = colors[tokenname]
    if #cols > 1 and color == cols[#cols][2] then
      cols[#cols][1] = cols[#cols][1] .. self.tokendata
    else
      cols[#cols + 1] = {self.tokendata, color}
    end
  end
  return cols
end

function EDITOR:PopulateMenu(menu)
  
end

function EDITOR:Paint()

end

WireTextEditor.Modes.JS = EDITOR