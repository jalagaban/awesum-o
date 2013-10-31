#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "world.h"
#include "command.h"
#include "tile.h"
#include "enemygui.h"
#include "storage.h"
#include <QString>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);

    TIMER = new QTimer(this);
    this->initWorld();
    this->loadPath("1\n2\n25\n160\n");
}

MainWindow::~MainWindow() {
    delete ui;

    World::getInstance().reset();
//    delete scene;
//    delete enemy;
}

void MainWindow::on_spawnBtn_clicked() {
    // run the POC test:
    // move objects around

    //create enemy command
    CreateCommand *createEn = new CreateCommand("walker", "walker");
    createEn->execute();

    //test display score
    int test = World::getInstance().getScore();
    QString q = QString::number(test);
    World::getInstance().enemyDeath();
    ui->scoreLbl->setText(q);

    Enemy *texas = World::getInstance().getEnemies().back();

    EnemyGUI *ranger = new EnemyGUI(this, texas, ui->graphicsView);
    ranger->setGeometry(texas->getX(), texas->getY(), 50, 50);
    ranger->setStyleSheet("QLabel { background-color : blue; border-style:dotted; border-width:1px; border-color: black; }");
    storage::getInstance().addEngui(ranger);
    ranger->show();

    //trying to animate on button click

//    TIMER->start();

    delete createEn;

}

void MainWindow::timerHit() {

    // get a collection of tiles
    //and enemies

    // move hte enemies

    // check and see if there are any enemies

    vector<Enemy*> toUpdate = World::getInstance().getEnemies();

    if (toUpdate.size() > 0) {
//        for (int a = 0; a < toUpdate.size(); ++a) {
//            toUpdate.at(a)->updatePosition();
//        }
        for (EnemyGUI* curEnemy : storage::getInstance().getEngui()) {
            curEnemy->getEnemyObj()->updatePosition();
            curEnemy->move(curEnemy->getEnemyObj()->getX(), curEnemy->getEnemyObj()->getY());
        }
    }

//    Enemy *frenemy = World::getInstance().getEnemies().back();
//    frenemy->updatePosition();

//    EnemyGUI *en = storage::getInstance().getEngui().back();
//    en->move(frenemy->getX(), frenemy->getY());
    // run the update method on each object

}
void MainWindow::initWorld() {

    TIMER->setInterval(100);
    connect(TIMER, &QTimer::timeout, this, &MainWindow::timerHit);

    // 16 x 10
    int x = 0;
    int y = 0;

    for (int i = 0; i < 160; ++i) {
        if(i == 16 || i == 32 || i == 48 || i == 64 || i == 80 || i == 96 || i == 112 || i == 128 || i == 144) {
            y += 50;
            x = 0;
        } else if (i != 0) {
            x += 50;
        }

        // create the object in the model and name it obj


        CreateCommand *createObj = new CreateCommand("tile", "tile");
        createObj->execute();
        Tile *obj = World::getInstance().getTiles().back();

        obj->setX(x);
        obj->setY(y);

        // create the entity labels
        Entity *tile = new Entity(this, obj, ui->graphicsView);
        // set the location of the entity
        tile->setScaledContents(true);
        tile->setGeometry(obj->getX(), obj->getY(), 50, 50);
        tile->setStyleSheet("QLabel { background-color : green; border-style:dotted; border-width:1px; border-color: black; }");
        tile->show();

        tile->setMouseTracking(true);

        delete createObj;

        // start the timer here...like you would add the beginning of a level.
        TIMER->start();
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
    for(int d = 0; d < lines.size(); ++d) {
        createPath(lines.at(d));
    }
}

void MainWindow::createPath(string cmd) {
    int slot = stoi(cmd);

    int x = 0;
    int y = 0;

    for (int i = 0; i < slot; ++i) {
        if(i == 16 || i == 32 || i == 48 || i == 64 || i == 80 || i == 96 || i == 112 || i == 128 || i == 144) {
            y += 50;
            x = 0;
        } else if (i != 0) {
            x += 50;
        }
    }

    vector<Tile*> tiles = World::getInstance().getTiles();

    for (int j = 0; j < tiles.size(); ++j) {
        Tile* tile = tiles.at(j);

        if(tile->getX() == x && tile->getY() == y) { // replace the thing!
            // get rid of the old tile
            delete tile;
            tiles.erase(tiles.begin()+j);

            // add the new tile!
            CreateCommand *createPath = new CreateCommand("path", "path");
            createPath->execute();

            Tile *path = World::getInstance().getTiles().back();
            path->setX(x);
            path->setY(y);

            // create the entity for it
            Entity *pathEntity = new Entity(this, path, ui->graphicsView);
            // set the location!
            pathEntity->setScaledContents(true);
            pathEntity->setGeometry(path->getX(), path->getY(), 50, 50);
            pathEntity->setStyleSheet("QLabel { background-color : brown; border-style:dotted; border-width:1px; border-color: black; }");
            pathEntity->show();

            pathEntity->setMouseTracking(true);

            delete createPath;
        }
    }
}

void MainWindow::on_buyBtn_clicked()
{
    int ifScore = World::getInstance().getScore();
    if (ifScore > 43){
    World::getInstance().towerBuy(43);
    int test = World::getInstance().getScore();
    QString q = QString::number(test);

    ui->scoreLbl->setText(q);
    }
}
