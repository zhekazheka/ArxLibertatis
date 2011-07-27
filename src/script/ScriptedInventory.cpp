
#include "script/ScriptedInventory.h"

#include <cstdlib>
#include <cstring>

#include "game/Inventory.h"
#include "graphics/data/Mesh.h"
#include "io/FilePath.h"
#include "scene/Interactive.h"
#include "scene/GameSound.h"
#include "script/ScriptUtils.h"

using std::string;
using std::free;
using std::malloc;
using std::memset;
using std::strcpy;

extern INTERACTIVE_OBJ * LASTSPAWNED;

namespace script {

namespace {

class InventoryCommand : public Command {
	
	class SubCommand : public Command {
		
		const string command;
		
	public:
		
		SubCommand(const std::string & name) : Command("inventory " + name, ANY_IO), command(name) { }
		
		inline const string & getCommand() { return command; }
		
	};
	
	typedef std::map<string, SubCommand *> Commands;
	Commands commands;
	
	void addCommand(SubCommand * command) {
		
		typedef std::pair<Commands::iterator, bool> Res;
		
		Res res = commands.insert(std::make_pair(command->getCommand(), command));
		
		if(!res.second) {
			LogError << "duplicate script inventory command name: " + command->getCommand();
			delete command;
		}
		
	}
	
	class CreateCommand : public SubCommand {
		
	public:
		
		CreateCommand() : SubCommand("create") { }
		
		Result execute(Context & context) {
			
			DebugScript("");
			
			INTERACTIVE_OBJ * io = context.getIO();
			
			if(io->inventory) {
				
				INVENTORY_DATA * id = io->inventory;
				
				for(long nj = 0; nj < id->sizey; nj++) {
					for(long ni = 0; ni < id->sizex; ni++) {
						
						INTERACTIVE_OBJ * item = id->slot[ni][nj].io;
						if(!item) {
							continue;
						}
						
						if(item->scriptload) {
							long tmp = GetInterNum(item);
							arx_assert(ValidIONum(tmp) && inter.iobj[tmp] == item);
							RemoveFromAllInventories(item);
							ReleaseInter(item);
							inter.iobj[tmp] = NULL;
						} else {
							item->show = SHOW_FLAG_KILLED;
						}
						
						id->slot[ni][nj].io = NULL;
					}
				}
				
				free(io->inventory);
			}
			
			io->inventory = (INVENTORY_DATA *)malloc(sizeof(INVENTORY_DATA));
			memset(io->inventory, 0, sizeof(INVENTORY_DATA));
			io->inventory->sizex = 3;
			io->inventory->sizey = 11;
			io->inventory->io = io;
			
			return Success;
		}
		
	};
	
	class SkinCommand : public SubCommand {
		
	public:
		
		SkinCommand() : SubCommand("skin") { }
		
		Result execute(Context & context) {
			
			context.getIO()->inventory_skin = fs::path::load(context.getWord());
			
			DebugScript(' ' << context.getIO()->inventory_skin);
			
			return Success;
		}
		
	};
	
	class PlayerAddFromSceneCommand : public SubCommand {
		
	public:
		
		PlayerAddFromSceneCommand() : SubCommand("playeraddfromscene") { }
		
		Result execute(Context & context) {
			
			string target = context.getWord();
			
			DebugScript(' ' << target);
			
			long t = GetTargetByNameTarget(target);
			if(t == -2) {
				t = GetInterNum(context.getIO());
			}
			if(!ValidIONum(t)) {
				ScriptWarning << "unknown target: " << target;
				return Failed;
			}
			
			RemoveFromAllInventories(inter.iobj[t]);
			inter.iobj[t]->show = SHOW_FLAG_IN_INVENTORY;
			if(!CanBePutInInventory(inter.iobj[t])) {
				PutInFrontOfPlayer(inter.iobj[t]);
			}
			
			return Success;
		}
		
	};
	
	class PlayerAddCommand : public SubCommand {
		
		const bool multi;
		
	public:
		
		PlayerAddCommand(const string & name, bool _multi) : SubCommand(name), multi(_multi) { }
		
		Result execute(Context & context) {
			
			string file = loadPath(context.getWord());
			
			if(FORBID_SCRIPT_IO_CREATION) {
				if(multi) {
					context.skipWord();
				}
				return Failed;
			}
			
			File_Standardize("graph/obj3d/interactive/items/" + file + ".teo", file);
			
			INTERACTIVE_OBJ * ioo = (INTERACTIVE_OBJ *)AddItem(file, IO_IMMEDIATELOAD);
			if(!ioo) {
				ScriptWarning << "could not add item \"" << file << '"';
				return Failed;
			}
			
			LASTSPAWNED = ioo;
			ioo->scriptload = 1;
			MakeTemporaryIOIdent(ioo);
			SendInitScriptEvent(ioo);
			
			if(multi) {
				
				float count = context.getFloat();
				
				DebugScript(' ' << file << ' ' << count);
				
				if(ioo->ioflags & IO_GOLD) {
					ioo->_itemdata->price = static_cast<long>(count);
				} else {
					ioo->_itemdata->maxcount = 9999;
					ARX_CHECK_SHORT(count);
					ioo->_itemdata->count = std::max(static_cast<short>(count), (short)1);
				}
				
			} else {
				DebugScript(' ' << file);
			}
			
			ioo->show = SHOW_FLAG_IN_INVENTORY;
			
			if(!CanBePutInInventory(ioo)) {
				PutInFrontOfPlayer(ioo);
			}
			
			return Success;
		}
		
	};
	
	class AddFromSceneCommand : public SubCommand {
		
	public:
		
		AddFromSceneCommand() : SubCommand("addfromscene") { }
		
		Result execute(Context & context) {
			
			string target = context.getWord();
			
			DebugScript(' ' << target);
			
			long t = GetTargetByNameTarget(target);
			if(t == -2) {
				t = GetInterNum(context.getIO());
			}
			if(!ValidIONum(t)) {
				ScriptWarning << "unknown target: " << target;
				return Failed;
			}
			
			if(ARX_EQUIPMENT_IsPlayerEquip(inter.iobj[t])) {
				ARX_EQUIPMENT_UnEquip(inter.iobj[0], inter.iobj[t], 1);
			} else {
				RemoveFromAllInventories(inter.iobj[t]);
			}
			
			inter.iobj[t]->scriptload = 0;
			inter.iobj[t]->show = SHOW_FLAG_IN_INVENTORY;
			
			long xx, yy;
			if(!CanBePutInSecondaryInventory(context.getIO()->inventory, inter.iobj[t], &xx, &yy)) {
				PutInFrontOfPlayer(inter.iobj[t]);
			}
			
			return Success;
		}
		
	};
	
	class AddCommand : public SubCommand {
		
		const bool multi;
		
	public:
		
		AddCommand(const string & name, bool _multi) : SubCommand(name), multi(_multi) { }
		
		Result execute(Context & context) {
			
			string file = loadPath(context.getWord());
			
			INTERACTIVE_OBJ * io = context.getIO();
			
			if(FORBID_SCRIPT_IO_CREATION || !io->inventory) {
				if(multi) {
					context.skipWord();
				}
				return Failed;
			}
			
			File_Standardize("graph/obj3d/interactive/items/" + file + ".teo", file);
			
			long count = -1;
			if(multi) {
				float val = context.getFloat();
				
				DebugScript(" \"" << file << "\" " << val);
				
				count = static_cast<long>(val);
				
			} else {
				DebugScript(" \"" << file << '"');
			}
			
			INTERACTIVE_OBJ * ioo = (INTERACTIVE_OBJ *)AddItem(file, IO_IMMEDIATELOAD);
			if(!ioo) {
				ScriptWarning << "could not add item \"" << file << '"';
				return Failed;
			}
			
			if(!count) {
				return Success;
			}
			
			LASTSPAWNED = ioo;
			ioo->scriptload = 1;
			MakeTemporaryIOIdent(ioo);
			SendInitScriptEvent(ioo);
			ioo->show = SHOW_FLAG_IN_INVENTORY;
			
			if(multi) {
				if(ioo->ioflags & IO_GOLD) {
					ioo->_itemdata->price = count;
				} else {
					ioo->_itemdata->maxcount = 9999;
					ARX_CHECK_SHORT(count);
					ioo->_itemdata->count = std::max(static_cast<short>(count), (short)1);
				}
			}
			
			long xx, yy;
			if(!CanBePutInSecondaryInventory(context.getIO()->inventory, ioo, &xx, &yy)) {
					PutInFrontOfPlayer(ioo);
			}
			
			return Success;
		}
		
	};
	
	class DestroyCommand : public SubCommand {
		
	public:
		
		DestroyCommand() : SubCommand("destroy") { }
		
		Result execute(Context & context) {
			
			DebugScript("");
			
			INTERACTIVE_OBJ * io = context.getIO();
			if(io->inventory) {
				if(SecondaryInventory == io->inventory) {
					SecondaryInventory = NULL;
				}
				free(io->inventory), io->inventory = NULL;
			}
			
			return Success;
		}
		
	};
	
	class OpenCommand : public SubCommand {
		
	public:
		
		OpenCommand() : SubCommand("open") { }
		
		Result execute(Context & context) {
			
			DebugScript("");
			
			if(SecondaryInventory != context.getIO()->inventory) {
				SecondaryInventory = context.getIO()->inventory;
				ARX_SOUND_PlayInterface(SND_BACKPACK);
			}
			
			return Success;
		}
		
	};
	
	class CloseCommand : public SubCommand {
		
	public:
		
		CloseCommand() : SubCommand("close") { }
		
		Result execute(Context & context) {
			
			DebugScript("");
			
			if(context.getIO()->inventory != NULL) {
				SecondaryInventory = NULL;
				ARX_SOUND_PlayInterface(SND_BACKPACK);
			}
			
			return Success;
		}
		
	};
	
public:
	
	InventoryCommand() : Command("inventory", ANY_IO) {
		addCommand(new CreateCommand);
		addCommand(new SkinCommand);
		addCommand(new PlayerAddFromSceneCommand);
		addCommand(new PlayerAddCommand("playeradd", false));
		addCommand(new PlayerAddCommand("playeraddmulti", true));
		addCommand(new AddFromSceneCommand);
		addCommand(new AddCommand("add", false));
		addCommand(new AddCommand("addmulti", true));
		addCommand(new DestroyCommand);
		addCommand(new OpenCommand);
		addCommand(new CloseCommand);
	}
	
	Result execute(Context & context) {
		
		string cmdname = context.getWord();
		
		// Remove all underscores from the command.
		cmdname.resize(std::remove(cmdname.begin(), cmdname.end(), '_') - cmdname.begin());
		
		Commands::const_iterator it = commands.find(cmdname);
		if(it == commands.end()) {
			ScriptWarning << "unknown inventory command: " << cmdname;
			return Failed;
		}
		
		return it->second->execute(context);
	}
	
};

class EquipCommand : public Command {
	
public:
	
	EquipCommand() : Command("equip", ANY_IO) { }
	
	Result execute(Context & context) {
		
		bool unequip = false;
		HandleFlags("r") {
			unequip = test_flag(flg, 'r');
		}
		
		string target = context.getWord();
		
		DebugScript(' ' << options << ' ' << target);
		
		long t = GetTargetByNameTarget(target);
		if(!ValidIONum(t)) {
			ScriptWarning << "unknown target: " << target;
			return Failed;
		}
		
		if(unequip) {
			INTERACTIVE_OBJ * oes = EVENT_SENDER;
			EVENT_SENDER = inter.iobj[t];
			Stack_SendIOScriptEvent(context.getIO(), SM_EQUIPOUT);
			EVENT_SENDER = oes;
			ARX_EQUIPMENT_UnEquip(inter.iobj[t], context.getIO());
		} else {
			INTERACTIVE_OBJ * oes = EVENT_SENDER;
			EVENT_SENDER = inter.iobj[t];
			Stack_SendIOScriptEvent(context.getIO(), SM_EQUIPIN);
			EVENT_SENDER = oes;
			ARX_EQUIPMENT_Equip(inter.iobj[t], context.getIO());
		}
		
		return Success;
	}
	
};

class WeaponCommand : public Command {
	
public:
	
	WeaponCommand() : Command("weapon", IO_NPC) { }
	
	Result execute(Context & context) {
		
		bool draw = context.getBool();
		
		DebugScript(' ' << draw);
		
		INTERACTIVE_OBJ * io = context.getIO();
		
		if(draw) {
			if(io->_npcdata->weaponinhand == 0) {
				AcquireLastAnim(io);
				FinishAnim(io, io->animlayer[1].cur_anim);
				io->animlayer[1].cur_anim = NULL;
				io->_npcdata->weaponinhand = -1;
			}
		} else {
			if(io->_npcdata->weaponinhand == 1) {
				AcquireLastAnim(io);
				FinishAnim(io, io->animlayer[1].cur_anim);
				io->animlayer[1].cur_anim = NULL;
				io->_npcdata->weaponinhand = 2;
			}
		}
		
		return Success;
	}
	
};

class SetWeaponCommand : public Command {
	
public:
	
	SetWeaponCommand() : Command("setweapon", IO_NPC) { }
	
	Result execute(Context & context) {
		
		INTERACTIVE_OBJ * io = context.getIO();
		
		io->GameFlags &= ~GFLAG_HIDEWEAPON;
		HandleFlags("h") {
			if(flg & flag('h')) {
				io->GameFlags |= GFLAG_HIDEWEAPON;
			}
		}
		
		string weapon = loadPath(context.getWord());
		
		DebugScript(' ' << options << ' ' << weapon);
		
		Prepare_SetWeapon(io, weapon);
		
		return Success;
	}
	
};

}

void setupScriptedInventory() {
	
	ScriptEvent::registerCommand(new InventoryCommand);
	ScriptEvent::registerCommand(new EquipCommand);
	ScriptEvent::registerCommand(new WeaponCommand);
	ScriptEvent::registerCommand(new SetWeaponCommand);
	
}

} // namespace script
