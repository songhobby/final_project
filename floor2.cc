#include <iostream>            
#include <cmath>
#include <algorithm>
#include "floor2.h"
#include "type.h"
#include <cstdlib>
#include <ctime>
#include <fstream>
#include "style.h"
#include "aaron.h"
#include "shade.h"
#include "drow.h"
#include "vampire.h"
#include "troll.h"
#include "goblin.h"

#include "restore_health.h"
#include "boost_atk.h"
#include "boost_def.h"
#include "poison_health.h" 
#include "wound_atk.h" 
#include "wound_def.h"
	
#include "cell.h"
#include "space.h"
#include "vertical_wall.h"
#include "horizontal_wall.h"
#include "door.h"
#include "tile.h"
#include "passage.h"
#include "stair.h"

#include "window.h"
#include "panel.h"
#include "observer.h"
#include "enemy.h"
#include "human.h"
#include "dwarf.h"
#include "halfling.h"
#include "elf.h"
#include "orc.h"
#include "merchant.h"
#include "dragon.h"
#include "nightelf.h"
#include "eason.h"
#include "worgen.h"
#include "forsaken.h"
#include "bloodelf.h"

#include "item.h"
#include "treasure.h"
#include "small_hoard.h"
#include "normal_hoard.h"
#include "dragon_hoard.h"
#include "merchant_hoard.h"

#include "buff.h" 
#include "display.h"
#include "visitexcept.h"
	using namespace std;
#include "curses.h"

class Enemy;


void D_Floor::clearFloor(bool cleanPlayer){
	//	cout << "enter clear" << endl;
	if (cleanPlayer){
		thePlayer = nullptr;
		Merchant::revenge = false;
	} else {
		if (thePlayer->getPlayerInfo().level == 5){
			throw 'w';
		}
		thePlayer->levelUp();
	}
	board.clear();
	theEnemy.clear();
	theChamber.clear();
//	cout << "out clear" << endl;
}
		

D_Floor::D_Floor(Display& display): theDisplay{display},height{25}, width{79}, stop{false} {}
		

void D_Floor::init(string file){ // set up the board according to the given floor in the file.
	//cout << "enter init" << endl;
	theDisplay.w = make_shared<Window>(file);
//	p = make_shared<Panel>(nullptr);
	ifstream f {file};
	string line;
	shared_ptr<Object> o;
	for (int i = 0; i < height; i++){
		vector<shared_ptr<Object>> arr;
		getline(f, line);
		for (int j = 0; j < width; j++){
			if (line[j] == '.') {
				o = make_shared<Tile>(j,i);
				//o->setAttributes(i, j, TILE, false, nullptr);
			} else if (line[j] == '-') {
				o = make_shared<Horizontal_Wall>(j,i);
			   // o->setAttributes(i, j, HORIZONTAL_WALL, false, nullptr);
			} else if (line[j] == '|') {
				o = make_shared<Vertical_Wall>(j,i);
				//o->setAttributes(i, j, VERTICAL_WALL, false, nullptr);
			} else if (line[j] == '+') {
				o = make_shared<Door>(j,i);
				//o->setAttributes(i, j, DOOR, false, nullptr);
			} else if (line[j] == '#') {
				o = make_shared<Passage>(j,i);
				//o->setAttributes(i, j, PASSAGE, false, nullptr);
			} else {
				o = make_shared<Space>(j,i);
			   // o->setAttributes(i, j, SPACE, false, nullptr);
			}
			//add more conditions to achieve read another type of floor (all set).
			
			//attach the neighbours.
//            if (i - 1 >= 0) {
//                board[i][j]->attach(board[i-1][j]);
//                if (j -1 >= 0){
//                    board[i][j]->attach(board[i-1][j-1]);
//                }
//                if (j +1 < width){
//                    board[i][j]->attach(board[i-1][j+1]);
//                }
//            }
//            if (i + 1 < height){
//                board[i][j]->attach(board[i+1][j]);
//                if (j - 1 >= 0){
//                    board[i][j]->attach(board[i+1][j-1]);
//                }
//                if (j + 1 < width) {
//                    board[i][j]->attach(board[i+1][j+1]);
//                }
//            }
//            if (j - 1 >= 0) {
//                board[i][j]->attach(board[i][j-1]);
//            }
//            if (j + 1 < width) {
//                board[i][j]->attach(board[i][j+1]);
//            }
			o->attach(theDisplay.w);
			arr.emplace_back(o);
		}
		board.emplace_back(arr);
	}
	//cout << "befor chamber" << endl;
	//detect chamber;
	for (int t = 0; t < 5; t++) {
		//cout << " here " << t<< endl;
		int row,col;
		do {
			row = getRandom(0, height - 1);
			col = getRandom(0, width - 1);
		//	cout << row <<" " << col << endl;
//			cout << board[row][col]->getPos().posx << " " << row <<" " << col << endl;
		} while (board[row][col]->getPos().style != TILE || (board[row][col]->getPos().isRead)); //check if it is read
		Chamber cham;
		setChamber(row, col, cham.c);
	//	cout << cham.c.size() << endl;
		for (auto &n: cham.c) {
			(*n)->getPos().chamber_num = t+1;
		}
		theChamber.emplace_back(cham);
	}
	if (!thePlayer) {
		selectPlayer();
	}
	spawnAction();
//cout << "out init" << endl;
}

void D_Floor::spawnAction(){
	setPlayer();
	int taken = thePlayer->getPos().chamber_num;
	setStair(taken); //since we have to make sure that the stair doesn't spawn in the same chamber with the player.
	for (int i = 0; i < 10; i++){
		setPotion();
	}
	for (int t = 0; t < 10; t++){
		setTreasure();//if dragon hoard is surround with stationary.
	}
	for (int j = 0; j < 20; j++){
		setEnemy();
	}
	for(int i = 0; i < 3; i++){
	    setNewEnemy();
	}
}

		
void D_Floor::selectPlayer(){
	int n = getRandom(0,4);
	int pos = getRandom(0,theChamber[n].c.size() - 1);
	Pos position = (*theChamber[n].c[pos])->getPos(); 
	while (true){
	 //put this part in to display later.
		char player_select;
		mvprintw(26, 0,"Choose your player: s(Shade), d(Drow), v(Vampire),  g(Goblin), t(Troll) , n(Nightelf) ");
		player_select = getch();
		if (player_select == 's'){
			thePlayer = make_shared<Shade>(position.posx, position.posy); //the hp atk .. is assigned in ctor.
		} else if (player_select == 'd') {
			thePlayer = make_shared<Drow>(position.posx, position.posy);
		} else if (player_select == 'v') {
			thePlayer = make_shared<Vampire>(position.posx, position.posy);
		} else if (player_select == 'g') {
			thePlayer = make_shared<Goblin>(position.posx, position.posy);
		} else if (player_select == 'n') {
			thePlayer = make_shared<NightElf>(position.posx, position.posy); 
		} else if (player_select == 'e') {
			thePlayer = make_shared<Eason>(position.posx, position.posy); 
		} else if (player_select == 'a') {
			thePlayer = make_shared<Aaron>(position.posx, position.posy);
		} else if (player_select == 't') {
			thePlayer = make_shared<Troll>(position.posx, position.posy);  //add auto ???
		} else {
			thePlayer = make_shared<Shade>(position.posx, position.posy);
			continue;
		}
		break;
	}
	thePlayer->getPos().chamber_num = n; //assign the chamber number to the player.
	theChamber[n].c.erase(theChamber[n].c.begin() + pos);
}

void D_Floor::setPlayer(){ // generate player.
	theDisplay.p = make_shared<Panel>(thePlayer.get()); //create a panel here
	thePlayer->attach(theDisplay.p); //attach panel to each player
	theDisplay.w->notify(*thePlayer);
	theDisplay.p->notify(*thePlayer);
	//cout << "out set player" << endl;
}


	void D_Floor::setChamber(int r, int c, vector<shared_ptr<Object>*>& arr) {
		//cout << "setChamber" << board[r][c]->getPos().isRead << endl;
		//cout << board[r][c]->getPos().style;
		board[r][c]->flip();
		arr.emplace_back(&board[r][c]);
		if (board[r-1][c]->getPos().style == TILE && !(board[r-1][c]->getPos().isRead)) {
			setChamber(r-1, c, arr);
		}
		if (board[r+1][c]->getPos().style == TILE && !(board[r+1][c]->getPos().isRead)) {
			setChamber(r+1,c, arr);
		}
		if (board[r][c-1]->getPos().style == TILE && !(board[r][c-1]->getPos().isRead)){ 
			setChamber(r,c-1, arr);
		}
		if (board[r][c+1]->getPos().style == TILE && !(board[r][c+1]->getPos().isRead)) { setChamber(r,c+1, arr);
		}
	}
		


	void D_Floor::setStair(int taken){ //generate stairway.
		//cout << "set Stair()" << endl;
		int n;
		do {
			n = getRandom(0,4);
		} while (n == taken);
		int pos = getRandom(0, theChamber[n].c.size() - 1);
		Pos position = (*theChamber[n].c[pos])->getPos();
		auto o = make_shared<Stair>(position.posx, position.posy);
		
		//o->setAttributes(position.posy, position.posx, STAIR, false, nullptr);
		//theStair = o;
		*(theChamber[n].c[pos]) = o;
		theDisplay.w->notify(*(*theChamber[n].c[pos]));
		theChamber[n].c.erase(theChamber[n].c.begin() + pos);
		//cout << "setStair works fine" << endl;
	}

	void D_Floor::setPotion(){ //generate potion

		//cout << "set potion" << endl;
		int n = getRandom(0, 4);
		int pos = getRandom(0, theChamber[n].c.size() - 1);
		int p = getRandom(1,6);
		Pos position = (*theChamber[n].c[pos])->getPos();
		shared_ptr<Potion> o;
		switch (p){
			case 1: 
				o = make_shared<Restore_Health>(position.posx, position.posy); //ctor should take in x y to set position.
				break;
			case 2:
				o = make_shared<Boost_Atk>(position.posx, position.posy);
				break;
			case 3:
				o = make_shared<Boost_Def>(position.posx, position.posy);
				break;
			case 4: 
				o = make_shared<Poison_Health>(position.posx, position.posy);
				break;
			case 5:
				o = make_shared<Wound_Atk>(position.posx, position.posy);
				break;
			case 6:
				o = make_shared<Wound_Def>(position.posx, position.posy);
				break;
		}
		//thePotion.emplace_back(o);
		*(theChamber[n].c[pos]) = o; // put this object in to the board.
		theDisplay.w->notify(*(*theChamber[n].c[pos]));
		theChamber[n].c.erase(theChamber[n].c.begin() + pos);
		//cout << "setPotion complete" << endl;
	}

	void D_Floor::setTreasure(){ //generate gold.
		int p = getRandom(1,8);
		int n = getRandom(0, 4);
	//	while (1){
			int pos = getRandom(0, theChamber[n].c.size() - 1);
			Pos position = (*theChamber[n].c[pos])->getPos();
			shared_ptr<Treasure> o;
			switch (p){
				case 1: 
				case 2:
				case 3:
				case 4: 
				case 5:
					o = make_shared<Normal_Hoard>(position.posx, position.posy);
					break;
				case 6:
				case 7: 
					o = make_shared<Small_Hoard>(position.posx, position.posy);
					break;
				case 8:
					auto dragon = make_shared<Dragon>(position.posx, position.posy);
					int randr, randc;
//					int gard = 8;
					do {
						randr = getRandom (-1, 1) + position.posy;
						if (randr == 0) {
							randc = getRandom (0, 1) * 2 -1 + position.posx;
						} else randc = getRandom (-1, 1) + position.posx;
			//			if (board[randr][randc]->getPos().isRead == true){//since we use this field for set Chamber, so here should be false. and this is to prevent dragon gets stuck.
			//				board[randr][randc]->getPos().isRead = false;
			//				gard--;
			//				if (gard == 0) break;
					} while (!dragon->visit(*board[randr][randc], MOVE));
					dragon->getPos().posx = randc;
					dragon->getPos().posy = randr;
					dragon->getHoardX() = position.posx;
					dragon->getHoardY() = position.posy;
				//	if (gard == 0) continue;
					board[randr][randc] = dragon;
					theDisplay.w->notify(*board[randr][randc]);

					o = make_shared<Dragon_Hoard>(position.posx, position.posy); //I expect the ctor of DH spawn a dragon here!!!!
					dragon->attach(o);
					theDragon.emplace_back(dragon);
					theDisplay.w->notify(*board[randr][randc]);
					break;
					}
			*(theChamber[n].c[pos]) = o;
			theDisplay.w->notify(*(*theChamber[n].c[pos]));
			theChamber[n].c.erase(theChamber[n].c.begin() + pos);
	//		break;
	}


	void D_Floor::setEnemy(){ //generate enemy.
		int n = getRandom(0, 4);
		int pos = getRandom(0, theChamber[n].c.size() - 1);
		int p = getRandom(1,18);
		Pos position = (*theChamber[n].c[pos])->getPos();
		shared_ptr<Enemy> o = nullptr;
		switch (p){
			case 1: 
			case 2:
			case 3:
			case 4: 
				o = make_shared<Human>(position.posx, position.posy);
				//o->setAttributes(position.posy, position.posx, HUMAN, false, nullptr);
				break;
			case 5:
			case 6:
			case 7: 
				o = make_shared<Dwarf>(position.posx, position.posy);
			   // o->setAttributes(position.posy, position.posx, DWARF, false, nullptr);
				break;
			case 8:
			case 9:
			case 10: 
			case 11:
			case 12:
				o = make_shared<Halfling>(position.posx, position.posy);
			   // o->setAttributes(position.posy, position.posx, HALFLING, false, nullptr);
				break;
			case 13: 
			case 14:
				o = make_shared<Elf>(position.posx, position.posy);
			  //  o->setAttributes(position.posy, position.posx, ELF, false, nullptr);
				break;
			case 15:
			case 16: 
				o = make_shared<Orc>(position.posx, position.posy);
			   // o->setAttributes(position.posy, position.posx, ORC, false, nullptr);
				break;
			case 17:
			case 18:
				o = make_shared<Merchant>(position.posx, position.posy);
			   // o->setAttributes(position.posy, position.posx, MERCHANT, false, nullptr);
				break;
		}
		o->getPos().chamber_num = n+1;	
		theEnemy.emplace_back(o);
		*(theChamber[n].c[pos]) = o;
		theDisplay.w->notify(*(*theChamber[n].c[pos]));
		theChamber[n].c.erase(theChamber[n].c.begin() + pos);
	}



	void D_Floor::setNewEnemy(){ //generate enemy.
		int n = getRandom(0, 4);
		int pos = getRandom(0, theChamber[n].c.size() - 1);
		int p = getRandom(1,3);
		Pos position = (*theChamber[n].c[pos])->getPos();
		shared_ptr<Enemy> o = nullptr;
		switch (p){
			case 1:
				o = make_shared<BloodElf>(position.posx, position.posy);
				break;
			case 2:
				o = make_shared<Worgen>(position.posx, position.posy);
				break;
    			case 3:
				o = make_shared<ForSaken>(position.posx, position.posy);
				break;

		}
		o->getPos().chamber_num = n+1;	
		theEnemy.emplace_back(o);

		*(theChamber[n].c[pos]) = o;
		theDisplay.w->notify(*(*theChamber[n].c[pos]));
		theChamber[n].c.erase(theChamber[n].c.begin() + pos);
		
	}




	void D_Floor::floorVisit(string s, Type type){
		//cout << "enter floorvisit" << endl;
		int r = thePlayer->getPos().posy;
	//	cout << "here!!" << endl;
		int c = thePlayer->getPos().posx;
		int target_r, target_c;
	//	cout << "here " << endl;
		if (s == "no") {
			target_r = r-1;
			target_c = c;
		} else if (s == "so") {
			target_r = r+1;
			target_c = c;
		} else if (s == "ea") {
			target_r = r;
			target_c = c+1;
		} else if (s == "we") {
			target_r = r;
			target_c = c-1;
		} else if (s == "ne"){
			target_r = r-1;
			target_c = c+1;
		} else if (s == "nw"){
			target_r = r-1;
			target_c = c-1;
		} else if (s == "se"){
			target_r = r+1;
			target_c = c+1;
		} else {
			target_r = r+1;
			target_c = c-1;
		}
		//cout << "row :" << target_r << "col: " << target_c << endl;
		bool isSuccess;
		try{
		   isSuccess = 
				thePlayer->visit(*board[target_r][target_c], type); //catch throw
		   if (thePlayer->getInfo().hp <= 0 && type == PICKUP){
			   throw 'd';
		   }
		}
		catch (VisitExcept& exc) {
			//cout << "here!!!!!" << endl;
			isSuccess = true;
			if (exc.state == "stair"){
				clearFloor(false);
				init();
				// reduce the gabage
			} else if (exc.state == "pickup_potion"){
					board[target_r][target_c] = make_shared<Tile>(target_c,target_r);
					//aaron have to return visitExcept with "string"
					//throw;		
			//	theDisplay.w->notify(*board[target_r][target_c]);
			} else if (exc.state == "pickup_gold"){
					board[target_r][target_c] = make_shared<Tile>(target_c,target_r);
			} else if(exc.state == "dragon_hoard"){
				 auto deadDragon = find(theDragon.begin(),theDragon.end(),board[target_r][target_c]);
			  	theDragon.erase(deadDragon);
				board[target_r][target_c] = make_shared<Tile>(target_c, target_r);
				 //theDisplay.w->notify(*board[target_r][target_c]);
				}
			else if (exc.state == "destroy") {
			board[target_r][target_c] = make_shared<Tile>(target_c, target_r);
			}
			else if (exc.state == "destroy2") {
			board[target_r][target_c] = make_shared<Space>(target_c, target_r);
			}
			else if (exc.state == "build") {
			board[target_r][target_c] = make_shared<Vertical_Wall>(target_c, target_r);
			}
			else if (exc.state == "build2") {
			board[target_r][target_c] = make_shared<Passage>(target_c, target_r);
			} else {
			// enemy is dead {
			     //delete enemy
			     auto deadEnemy = find(theEnemy.begin(),theEnemy.end(),board[target_r][target_c]);
			     theEnemy.erase(deadEnemy);
				if (exc.state == "small_hoard") {
					board[target_r][target_c] = make_shared<Tile>(target_c, target_r);
				} else if (exc.state == "merchant_hoard"){
					board[target_r][target_c] = make_shared<Merchant_Hoard>(target_c, target_r);	
				}else if (exc.state == "normal_hoard"){
					while (exc.num > 1) {
					board[target_r][target_c] = make_shared<Normal_Hoard>(target_c, target_r);
					int randr, randc;
					do {
						randr = getRandom (-1, 1) + target_r;
						if (randr == 0) {
							randc = getRandom (0, 1) * 2 -1 + target_c;
						} else randc = getRandom (-1, 1) + target_c;
					} while (!board[target_r][target_c]->visit(*board[randr][randc], MOVE));
						swap(board[randr][randc]->getPos().posx, board[target_r][target_c]->getPos().posx);
						swap(board[randr][randc]->getPos().posy, board[target_r][target_c]->getPos().posy);
						swap(board[randr][randc], board[target_r][target_c]);
						theDisplay.w->notify(*board[randr][randc]);
						--exc.num;
					}
						board[target_r][target_c] = make_shared<Normal_Hoard>(target_c, target_r);
						
				} else {// merchant_hoard


//merchant dragon drop 
				}
			}
		}

		theDisplay.w->notify(*board[target_r][target_c]);
		if (!isSuccess) {
				//cout << "false" << endl;
		} else {
			if (type == MOVE){
				thePlayer->getPos().chamber_num = board[target_r][target_c]->getPos().chamber_num;
				//cout << "true" << endl;
				thePlayer->getPos().posx = target_c;
				thePlayer->getPos().posy = target_r;
			//	theDisplay.w->notify(*thePlayer);
			//	theDisplay.p->notify(*thePlayer);
				//	cout << *this;
			} else if (type == ATTACK) {
				theDisplay.p->notify(*thePlayer);
			} else {
				//cout << "PICKUP condition" << endl;
			}
		}
		theDisplay.w->notify(*thePlayer);
		theDisplay.w->notify(*board[target_r][target_c]);
		theDisplay.p->notify(*thePlayer);

	//enemy random move.
		sort(theEnemy.begin(), theEnemy.end(), compare);
		if (!stop) {
			int player_r = thePlayer->getPos().posy;
			int player_c = thePlayer->getPos().posx;
			bool playeraround = false;

		    //dragon case
		 for(int i = 0; i < theDragon.size(); i++){
		    int dragonr = theDragon[i]->getPos().posy;
		    int dragonc = theDragon[i]->getPos().posx;
		    int hoardr = theDragon[i]->getHoardY();
		    int hoardc = theDragon[i]->getHoardX();
			if ((abs(player_r - dragonr) <= 1 && abs(player_c - dragonc) <= 1) ||
			    (abs(player_r - hoardr) <= 1 && abs(player_c - hoardc) <= 1)){
				playeraround = true;
				try	{
				//cout << "player is attacked" << endl;
						theDragon[i]->visit(*thePlayer, ATTACK);
				}
				catch(VisitExcept & exc){
					if (exc.state == "deadplayer"){
						playeraround = true;
						throw 'd';
					}
				}
			}
			if(playeraround){
				theDisplay.p->notify(*thePlayer);
			}
		 }
		    // other enemy
			for (int i = 0; i < theEnemy.size(); i++){
		    playeraround = false;
				int r = theEnemy[i]->getPos().posy;
				int c = theEnemy[i]->getPos().posx;
				if (abs(player_r - r) <= 1 && abs(player_c - c) <= 1){
					playeraround = true;
					try	{
					//cout << "player is attacked" << endl;
							theEnemy[i]->visit(*thePlayer, ATTACK);
					}
					catch(VisitExcept & exc){
						if (exc.state == "deadplayer"){
							playeraround = true;
							throw 'd';
						}
					}
				}
				if (!playeraround){
					vector<bool> possibility(8, false);
						enemyMove(i, possibility);
				} else {
					theDisplay.p->notify(*thePlayer);
				}
			}
		}
	//	theDisplay.w->notify(*thePlayer);
	//	theDisplay.w->notify(*board[target_r][target_c]);
//		theDisplay.p->notify(*thePlayer);
		//		
		//theDisplay.display();
		thePlayer->getPlayerInfo().action = "";
	//cout << "enemy random move complete" << endl;
	}
void D_Floor::pause(){
	if (stop) stop = false;
	else stop = true;
}

bool D_Floor::enemyMove(int n, vector<bool>& possibility) {
	int r = theEnemy[n]->getPos().posy;
	int c = theEnemy[n]->getPos().posx;
	int target_r, target_c;
	int i = getRandom(1,8); //0 means the enemy don't move this round(don't know if it is necessary).
	if (i == 1) {
		target_r = r-1;
		target_c = c;
		possibility[i] = true;
	} else if (i == 2) {
		target_r = r+1;
		target_c = c;
		possibility[i] = true;
	} else if (i == 3) {
		target_r = r;
		target_c = c+1;
		possibility[i] = true;
	} else if (i == 4) {
		target_r = r;
		target_c = c-1;
		possibility[i] = true;
	} else if (i == 5){
		target_r = r-1;
		target_c = c+1;
		possibility[i] = true;
	} else if (i == 6){
		target_r = r-1;
		target_c = c-1;
		possibility[i] = true;
	} else if (i == 7){
		target_r = r+1;
		target_c = c+1;
		possibility[i] = true;
	} else {
		target_r = r+1;
		target_c = c-1;
		possibility[i] = true;
	}
	Pos playerpos = thePlayer->getPos();
	if (theEnemy[n]->getPos().style != MERCHANT){//here we make merchant chase the player only when it's in revange mode.
		if (distance(playerpos.posy, playerpos.posx, target_r, target_c) < 
				distance(playerpos.posy, playerpos.posx, r,c) || 
				(thePlayer->getPos().chamber_num != theEnemy[n]->getPos().chamber_num)){ //only trace when in same chamber  move when distance.
			if (theEnemy[n]->visit(*board[target_r][target_c], MOVE)){
				swap(theEnemy[n]->getPos().posx, board[target_r][target_c]->getPos().posx);
				swap(theEnemy[n]->getPos().posy, board[target_r][target_c]->getPos().posy);
				swap(board[r][c], board[target_r][target_c]);
				theDisplay.w->notify(*theEnemy[n]);
				theDisplay.w->notify(*board[r][c]);
				return true;
			} else if (!(possibility[1] &&  possibility[2] &&  possibility[3] &&
				   	possibility[4] && possibility[5] && possibility[6] &&
				   	possibility[7] && possibility[8])) {
				enemyMove(n, possibility);
			} else { //in this case the enemy is surrounded by other stuff.
				return false;
			}
		} else {
			enemyMove(n, possibility);
		}
	} else {
		if (Merchant::revenge){
			if (distance(playerpos.posy, playerpos.posx, target_r, target_c) < 
					distance(playerpos.posy, playerpos.posx, r,c) || 
					(thePlayer->getPos().chamber_num != theEnemy[n]->getPos().chamber_num)){ //only trace when in same chamber  move when distance.
				if (theEnemy[n]->visit(*board[target_r][target_c], MOVE)){
					swap(theEnemy[n]->getPos().posx, board[target_r][target_c]->getPos().posx);
					swap(theEnemy[n]->getPos().posy, board[target_r][target_c]->getPos().posy);
					swap(board[r][c], board[target_r][target_c]);
					theDisplay.w->notify(*theEnemy[n]);
					theDisplay.w->notify(*board[r][c]);
					return true;
				} else if (!(possibility[1] &&  possibility[2] &&  possibility[3] &&
					   	possibility[4] && possibility[5] && possibility[6] &&
					   	possibility[7] && possibility[8])) {
					enemyMove(n, possibility);
				} else { //in this case the enemy is surrounded by other stuff.
					return false;
				}
			} else {
				enemyMove(n, possibility);
			}
		} else {
		   if 	(theEnemy[n]->visit(*board[target_r][target_c], MOVE)){
					swap(theEnemy[n]->getPos().posx, board[target_r][target_c]->getPos().posx);
					swap(theEnemy[n]->getPos().posy, board[target_r][target_c]->getPos().posy);
					swap(board[r][c], board[target_r][target_c]);
					theDisplay.w->notify(*theEnemy[n]);
					theDisplay.w->notify(*board[r][c]);
					return true;
				} else if (!(possibility[1] &&  possibility[2] &&  possibility[3] &&
					   	possibility[4] && possibility[5] && possibility[6] &&
					   	possibility[7] && possibility[8])) {
			enemyMove(n, possibility);
				} else { //in this case the enemy is surrounded by other stuff.
				return false;
				}
		}
	}
    return false; 
}

void D_Floor::windowPreprocessor(string message, int r, int c){
	ifstream f {message};
	ofstream after {"modified_window.txt"};
	string line;
	for (int i = 0; i < height; i++){
		getline(f, line);
		if (i == r){
			int gold = thePlayer->getPlayerInfo().gold;
			string s = "Your score is: " + to_string(gold);
			line.replace (c,s.length(),s);
			after << line;
			after << endl;
		} else {
			after << line;
			after << endl;	
		}
	}
	after.close();
}
	
ostream &operator<<(ostream &out, const D_Floor &f){
	out << *f.theDisplay.w; //this is window.
	out << *f.theDisplay.p; //this is plane.
	return out;
}

string D_Floor::outPut(){
	string rs = theDisplay.w->outPut() +
		theDisplay.p->outPut();
	return rs;
}





