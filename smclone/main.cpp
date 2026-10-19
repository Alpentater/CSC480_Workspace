//Qapplication is what is used for GUI and QMessageBox is for pop-ups, like errors and the like.
#include <QApplication>
#include <QMessageBox>

//Handles the connection and initialization of the SQLite database.
#include "database/DatabaseManager.h"

//Main application window cotaining the scheduler UI
#include "ui/MainWindow.h"

//Main entry point for a standard c++ program.
int main(int argc, char *argv[]){
  //Creating the Qt application object. argc and argv are for accepting command line arguments
  QApplication app(argc, argv);

  //name and company name of my program metadata :)
  QApplication::setApplicationName("Alpenmonster");
  QApplication::setOrganizationName("Alex Carpenter");
  // Create the database manager this object is responsible for opening and managing the SQLite database.
  DatabaseManager database;
  //If this fails to initialize, return the error AND a 1 to indicate failure to initialize the program.
  if(!database.initialize()){
    QMessageBox::critical(nullptr, "Database Error", "The scheduler database could not be opened." + database.lastError());
    return 1;
  }

  //Setting the window stuff, this includes the size and the visibility, etc.
  MainWindow window(&database);
  window.resize(1450, 850);
  window.show();
  //On success, 'execute' the application! (Run it)
  return app.exec();
}