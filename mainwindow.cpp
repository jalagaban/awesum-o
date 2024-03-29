#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "world.h"
#include "command.h"
#include "tile.h"
#include "enemygui.h"
#include "storage.h"
#include <QString>
#include <fstream>
#include <cassert>
#include "bulletgui.h"
#include <QString>
#include <QMessageBox>
#include "helpform.h"
#include "gameover.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ui->btnAddTower->setCheckable(true);
    ui->btnAddTower->setEnabled(false);
    ui->btnStartLevel->setEnabled(false);

    ui->graphicsView->setStyleSheet("background-image: url(:/images/background.jpg)");

    TIMER = new QTimer(this);
}

MainWindow::~MainWindow() {
    storage::getInstance().reset();
    delete ui;
    World::getInstance().reset();
}

void MainWindow::timerHit() {
    // get the collection of enemies
    ui->btnLoadLevel->setEnabled(false);

    int i = World::getInstance().getScore();
    int lf = World::getInstance().getLives();
    int cr = storage::getInstance().getWavecreator();
    bool firstGo = storage::getInstance().getFG();
    bool cheatset = storage::getInstance().getCheat();
    if (cheatset == true) {
        World::getInstance().setLives(9999999999);

    }
    if (lf == 0 && firstGo) {
        storage::getInstance().setEnd(true);

    }
    if (lf == 0 && storage::getInstance().getEnd() && firstGo){
        // disable save and load
        ui->btnSave->setEnabled(false);
        ui->btnLoad->setEnabled(false);

        gameOver *over = new gameOver();
        over->show();
        storage::getInstance().setFG(false);

    }
    QString s;
    s.setNum(i,10);
    ui->scoreLbl->setText(s);
    s.setNum(lf,10);
    ui->lifeLabel->setText(s);
    vector<Enemy*> *toUpdate = World::getInstance().getEnemies();
    vector<EnemyGUI*> *engui = storage::getInstance().getEngui();

    //endgame screen


    // see if there are any enemies
    if (toUpdate->size() > 0) {
        //check to see if the enemys are out of bounds
        for (unsigned int forloop = 0; forloop < engui->size();++forloop){
            EnemyGUI *deathTest = engui->at(forloop);
            int yTest = deathTest->getEnemyObj()->getY();
            if (yTest >= 500) {
                int deadId = deathTest->getEnemyObj()->getId();
                engui->at(forloop)->deleteLater();
                World::getInstance().removeEnemy(deadId);
                engui->erase(engui->begin()+forloop);
                World::getInstance().decLives();

            }


        }
        // for each enemy, update it's position

        for (unsigned int a = 0; a < engui->size(); ++a) {
            EnemyGUI *curEnemy = engui->at(a);
            curEnemy->getEnemyObj()->updatePosition();
            curEnemy->updateDirection();
            curEnemy->setPic();
            curEnemy->move(curEnemy->getEnemyObj()->getX(), curEnemy->getEnemyObj()->getY());
        }
    }

    // TODO M1 : get the towers and run their update methods

    vector<Tile*> *tiles = World::getInstance().getTiles();

    if (tiles->size() > 0) {
        vector<Entity*> *entities = storage::getInstance().getEntities();

        for(unsigned int g = 0; g < entities->size(); ++g) {
            if(dynamic_cast<towerTile*>(entities->at(g)->getTile())) {
                towerTile* curTile = dynamic_cast<towerTile*>(entities->at(g)->getTile());

                // check and see if we can launch a new bullet yet!
                if(curTile->getCurFire() >= curTile->getFireSpeed()) {

                    curTile->launchMaybe();
                    Enemy *e = curTile->getNewTarget();
                    if(e != NULL) {
                        // create the bullet here!!
                        int curid = e->getId();
                        stringstream forCreate;
                        forCreate << to_string(curTile->getX()) << " " << to_string(curTile->getY()) << " bullet " << curid << " -1" << endl;
                        doCreate(forCreate);

                        World::getInstance().getBullets()->back()->setDamage(curTile->getDamage());

                        // reset the target
                        curTile->resetNewTarget();
                        curTile->setCurFire(0);
                    }
                } else {
                    curTile->updateCurFire();
                }
            }
        }
    }

    // TODO M1 : get the bullets and run their update methods

    if (World::getInstance().getBullets()->size() > 0) {
        vector<BulletGUI*> *bullets = storage::getInstance().getBgui();

        for (int q = 0; q < bullets->size(); ++q) {
            BulletGUI *guiBullet = bullets->at(q);
            Bullet *modelBullet = guiBullet->getBulletObj();

            // check and see if the bullet has a live target
            if(World::getInstance().getEnemyById(modelBullet->getTargetId()) != NULL) {
                // it has a live target

                modelBullet->updatePosition();
                cout << "Updated bullet " << to_string(modelBullet->getId()) << endl;

                // update the GUI position of the bullet
                guiBullet->move(modelBullet->getX(), modelBullet->getY());

                //------HANDLE COLLISION-------//

                // check and see if the bullet is in the target (collision has happened)
                if(modelBullet->isInTarget()) {
                    Enemy *target = World::getInstance().getEnemyById(modelBullet->getTargetId());
                    target->hit(modelBullet->getDamage());

                    int id = modelBullet->getTargetId();

                    // check and see if it's dead
                    if(target->deathUpdate() >= 0) {
                        for(unsigned int t = 0; t < engui->size(); ++t) {
                            if(engui->at(t)->getEnemyObj()->getId() == id) {
                                engui->at(t)->deleteLater();
                                World::getInstance().removeEnemy(id);
                                engui->erase(engui->begin()+t);
                            }
                        }
                    }

                    bullets->erase(bullets->begin()+q);
                    delete guiBullet;
                    --q; // decrement q since we just removed something from it!

                    // delete the model bullet
                    World::getInstance().removeBullet(modelBullet->getId());
                }

            } else {
                // the bullet no longer has a target - get rid of the bullet
                // TODO: make the bullet exit the screen instead of deleteing it

                bullets->erase(bullets->begin()+q);
                guiBullet->deleteLater();
                --q; // decrement q since we just removed something from it!

                // delete the model bullet
                World::getInstance().removeBullet(modelBullet->getId());

                // make sure that we don't skip over anything!!
            }


        }
    }
    bool booltester = storage::getInstance().getStarted();
    int k = 10;
    bool endTest2 = storage::getInstance().getEnd();
    if (endTest2){
        k = 1;

    }
    if ((cr % (k * storage::getInstance().getDiff())) == 0 && booltester == true) {
        stringstream forCreate;
        forCreate << string("0 0 enemy walker -1") << endl;
        doCreate(forCreate);
        storage::getInstance().incCreator();

    } else if ((cr % (15 * storage::getInstance().getDiff())) == 0 && booltester == true) {
        stringstream forCreate;
        forCreate << string("0 0 enemy yolo -1") << endl;
        doCreate(forCreate);
        storage::getInstance().incCreator();

    } else if((cr % (25 * storage::getInstance().getDiff())) == 0 && booltester == true){
        stringstream forCreate;
        forCreate << string("0 0 enemy sergeant -1") << endl;
        doCreate(forCreate);
        storage::getInstance().incCreator();
    } else {
        storage::getInstance().incCreator();
    }

}//end of timerHit

// return the correct coordinates for the specified slot
int MainWindow::getSlotCoord(int slotNum, string coordType) {
    int x = 0;
    int y = 0;

    for (int i = 0; i < slotNum; ++i) {
        if(i == 16 || i == 32 || i == 48 || i == 64 || i == 80 || i == 96 || i == 112 || i == 128 || i == 144) {
            y += 50;
            x = 0;
        } else if (i != 0) {
            x += 50;
        }
    }

    if (coordType == "y") {
        return y;
    } else if (coordType == "x") {
        return x;
    }

    return 0;
}

// initialize the world by loading all the normal tiles and setting up the timers.
void MainWindow::initWorld() {

    TIMER->setInterval(40);
    connect(TIMER, &QTimer::timeout, this, &MainWindow::timerHit);

    int x = 0;
    int y = 0;

    for (int i = 1; i <= 160; ++i) {
        x = getSlotCoord(i, "x");
        y = getSlotCoord(i, "y");

        stringstream forCreate;
        forCreate << to_string(x) << string(" ") << to_string(y) << string(" tile tile -1") << endl;

        doCreate(forCreate);

        // start the timer here...like you would add the beginning of a level.
        TIMER->start();

        // enable the buttons
        ui->btnAddTower->setEnabled(true);
        ui->btnStartLevel->setEnabled(true);
    }
}

void MainWindow::loadPath(string pathString) {
    vector<string> lines;

    stringstream paths(pathString);

    string line;
    getline(paths, line);

    while (paths.rdbuf()->in_avail() != 0) {
        lines.push_back(line);
        getline(paths, line);
    }

    lines.push_back(line);

    // load the path(s)
    for(unsigned int d = 0; d < lines.size(); ++d) {
        createPath(lines.at(d));
    }
}

void MainWindow::createPath(string cmd) {
    int slot = stoi(cmd);

    int x = 0;
    int y = 0;

    x = getSlotCoord(slot, "x");
    y = getSlotCoord(slot, "y");

    vector<Entity*> *entities = storage::getInstance().getEntities();

    // find and remove the entity from the slot that the path would be on
    for(unsigned int d = 0; d < entities->size(); ++d) {
        Entity *entity = entities->at(d);

        if(entity->getTile()->getX() == x && entity->getTile()->getY() == y) { // replace the thing!
            // get rid of the old entity
            entity->deleteLater();
            entities->erase(entities->begin()+d);
            return;
        }
    }
}

void MainWindow::on_btnLoadLevel_clicked() {
    // clear the world
    World::getInstance().reset();

    // load the specific level
    this->initWorld(); // load the all the tiles to set the background

    // load the path over the world
    this->loadPath("1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n32\n48\n47\n46\n45\n44\n43\n42\n41\n40\n39\n38\n37\n36\n35\n34\n33\n47\n49\n65\n66\n67\n68\n69\n70\n71\n72\n73\n74\n75\n76\n77\n78\n79\n80\n96\n97\n98\n99\n100\n101\n102\n103\n104\n105\n106\n107\n108\n109\n110\n111\n112\n113\n129\n130\n131\n132\n133\n134\n135\n136\n137\n138\n139\n140\n141\n142\n143\n144\n160");

    ui->btnLoadLevel->setEnabled(false);
}

//Start the game
void MainWindow::on_btnStartLevel_clicked() {
    // disable the buttons so the user can't start more things!
    ui->btnStartLevel->setEnabled(false);
    storage::getInstance().setStarted();

    // load the enemies here!!

    stringstream forCreate;
    forCreate << string("0 0 enemy walker -1") << endl;
    doCreate(forCreate);

    ui->btnLoad->setEnabled(true);
    ui->btnSave->setEnabled(true);
}

// enable the buying of a tower
void MainWindow::on_btnAddTower_clicked() {
}

// create a tower
void MainWindow::createTower(int x, int y) {
    if (World::getInstance().getScore() >= 125){
        stringstream forCreate;
        forCreate << to_string(x) << string(" ") << to_string(y) << string(" tile tower -1") << endl;
        doCreate(forCreate);
        World::getInstance().towerBuy(125);
        QString b;
        b.setNum(World::getInstance().getScore(),10);
        ui->scoreLbl->setText(b);
    }
}

// checks to see if the user has selected to create towers
bool MainWindow::getCanCreateTower() {
    return ui->btnAddTower->isChecked();
}

// creates an object in the model and it's corresponding GUI object
void MainWindow::doCreate(stringstream& cmd) {
    string type, specific, id;
    int x, y;
    cmd >> x;
    cmd >> y;
    cmd >> type;
    cmd >> specific;
    cmd >> id;
    bool tyletestfix = false;

    if (cmd) {
        // create the object in the model
        if(type == "bullet") {
            CreateCommand *createObj = new CreateCommand("bullet");
            createObj->execute();
            delete createObj;
        } else {
            if(type == "tile") {
                if (y % 100 != 0 && y != 0)
                {
                    if(!(y == 50 && x == 750))
                    {
                        if(!(y == 150 && x == 0))
                        {
                            if(!(y == 250 && x == 750))
                            {
                                if(!(y== 350 && x ==0))
                                {
                                    if (!(y==450 && x ==750))
                                    {
                                        if (!(y==550 && x == 750)){
                                            CreateCommand *createObj = new CreateCommand(specific);
                                            createObj->execute();
                                            delete createObj;
                                            tyletestfix = true;
                                        }
                                    }
                                }
                            }
                        }
                    }

                }

            } else {
                CreateCommand *createObj = new CreateCommand(specific);
                createObj->execute();
                delete createObj;
            }

        }

        // build the string for the style
        //        string style("QLabel { background-color : " + image + "; border-style:dotted; border-width:1px; border-color: black; }");
        //        QString forStyle(style.c_str()); // convert it so the method will accept the variable

        if (type == "tile") {
            if (tyletestfix){
                Tile *obj = World::getInstance().getTiles()->back();

                obj->setX(x);
                obj->setY(y);
                //            obj->setImage(image);

                // create the GUI component
                Entity *tile = new Entity(this, obj, ui->graphicsView);
                tile->setScaledContents(true);
                tile->setGeometry(obj->getX(), obj->getY(), 50, 50);
                tile->setPic();
                //tile->setStyleSheet(forStyle);
                tile->show();

                tile->setMouseTracking(true); // turn mouse tracking on for mouse over stuff

                storage::getInstance().addEntity(tile);
            }

        } else if (type == "enemy") {

            Enemy *texas = World::getInstance().getEnemies()->back();

            // no need to set x or y since they start at 0 except when loading!
            texas->setX(x);
            texas->setY(y);

            if(stoi(id) != -1) {
                texas->setId(stoi(id));
                Enemy::setNextId(stoi(id) + 1);
            }

            // create the GUI component
            EnemyGUI *ranger = new EnemyGUI(this, texas, ui->graphicsView);
            ranger->setScaledContents(true);
            ranger->setPic();
            ranger->setGeometry(texas->getX(), texas->getY(), 50, 50);
            //ranger->setStyleSheet(forStyle);
            ranger->show();

            storage::getInstance().addEngui(ranger);

        } else if (type == "bullet") {
            Bullet *obj = World::getInstance().getBullets()->back();

            obj->setX(x);
            obj->setY(y);
            obj->setTarget(stoi(specific));

            BulletGUI *blt = new BulletGUI(this, obj, ui->graphicsView);
            blt->setGeometry(obj->getX(), obj->getY(), 10, 10);
            blt->show();

            storage::getInstance().addBgui(blt);
        }
    }

}

void MainWindow::load() {
    ifstream infile("saveData.awt");

    // see if the file exists
    if(infile.good()) {

        // delete all the labels.
        storage::getInstance().reset();

        // reset the world
        World::getInstance().reset();



        string line;
        int score, lives;

        infile >> score >> lives;

        World::getInstance().initScore(score);

        World::getInstance().setLives(lives);

        while(getline(infile, line)) {
            stringstream ss(line);
            doCreate(ss);
        }

    } else {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Error!");
        msgBox.setText("Save file does not exist. You must first create a save before loading.");
        msgBox.exec();
    }
}

void MainWindow::save() {
    // get a file name to save to
    char* filename = "saveData.awt";

    ofstream fout;
    fout.open(filename, ios::out);

    fout << World::getInstance().getScore() << " " << World::getInstance().getLives() << endl;

    vector<Tile*> *tiles = World::getInstance().getTiles();

    // get the model objects in the game
    for(unsigned int d = 0; d < tiles->size(); ++d) { // iterate over them and save them to a file
        tiles->at(d)->save(fout);
        fout << " -1" << endl;
    }

    // save the bullets
    vector<Bullet*> *bullets = World::getInstance().getBullets();

    for(unsigned int c = 0; c < bullets->size(); ++c) {
        bullets->at(c)->save(fout);
        fout << " -1" << endl;
    }


    // save the enemies
    vector<Enemy*> *enemies = World::getInstance().getEnemies();

    for(unsigned int g = 0; g< enemies->size(); ++g) {
        enemies->at(g)->save(fout);
        fout << endl;
    }

    // close the file after we are done writing it
    fout.close();
}

//Create server
void MainWindow::on_btnServer_clicked()
{
    server = new AwesumServer;
    if (server->getConnection())
    {
        server->activateWindow();
        server->show();
    }
    else
    {
        QMessageBox::critical(this, "Uh oh", "Could not start socket.");
    }
}

//Connect to existing server
void MainWindow::on_btnClient_clicked()
{
    client = new AwesumeClient;
    if (server->getConnection())
    {
        client->activateWindow();
        client->show();
    }
    else
    {
        QMessageBox::critical(this, "Uh oh", "Could not connect.");
    }
}

void MainWindow::on_helpBtn_toggled(bool checked)
{
    helpForm *helpform1 = new helpForm();
    if (checked) {


        helpform1->show();
    } else {
        helpform1->deleteLater();
    }

}

void MainWindow::on_btnSave_clicked()
{
    // call save method
    this->save();
}

void MainWindow::on_btnLoad_clicked() {
    this->load();
}

void MainWindow::on_diff1BTN_toggled(bool checked)
{
    if (checked){
        ui->diff2BTN->setChecked(false);
        ui->diff3BTN->setChecked(false);
        storage::getInstance().setDiff(3);
        World::getInstance().setPrecheat(World::getInstance().getLives());
        storage::getInstance().setCheatadj(true);
       // ui->btnAddTower->setEnabled(true);
        //ui->btnStartLevel->setEnabled(true);
        ui->btnLoadLevel->setEnabled(true);
    } else {
        World::getInstance().setLives(World::getInstance().getPrecheat());
        storage::getInstance().setCheatadj(false);
    }
}

void MainWindow::on_diff2BTN_toggled(bool checked)
{
    if (checked){
        ui->diff1BTN->setChecked(false);
        ui->diff3BTN->setChecked(false);
        storage::getInstance().setDiff(2);
       // ui->btnAddTower->setEnabled(true);
       // ui->btnStartLevel->setEnabled(true);
        ui->btnLoadLevel->setEnabled(true);
    }
}

void MainWindow::on_diff3BTN_toggled(bool checked)
{
    if (checked){
        ui->diff2BTN->setChecked(false);
        ui->diff1BTN->setChecked(false);
        storage::getInstance().setDiff(1);
       // ui->btnAddTower->setEnabled(true);
        //ui->btnStartLevel->setEnabled(true);
        ui->btnLoadLevel->setEnabled(true);
    }
}
