// This is where scripts' loading functions should be declared:
void AddSC_DC_test();
void AddSC_DC_teleporter();
void AddSC_DC_composer();
void AddSC_DC_items();
void AddSC_DC_commandscript();

// The name of this function should match:
// void Add${NameOfDirectory}Scripts()
void AddDreamCoreScripts()
{
    AddSC_DC_test();
    AddSC_DC_teleporter();
    AddSC_DC_composer();
    AddSC_DC_items();
    AddSC_DC_commandscript();
}
