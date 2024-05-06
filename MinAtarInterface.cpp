#include "MinAtarInterface.hpp"
#include <iostream>

// Compile with -DMINATAR_PATH=\"<path>\" to change the path
#ifdef MINATAR_PATH
#define MINATAR_PATH_ MINATAR_PATH
#else
#define MINATAR_PATH_ "MinAtar/" // Default path
#endif

using namespace std;

MinAtarInterface::MinAtarInterface(string gameName,
				   float stickyAction,
				   bool difficultyRamping,
				   size_t randomSeed,
				   bool initPy,
				   bool finalizePy) :
   initPy_{initPy},
   finalizePy_{finalizePy},
   gameName_{gameName}
{
   if (gameName != "asterix" and
       gameName != "breakout" and
       gameName != "freeway" and
       gameName != "seaquest" and
       gameName != "space_invaders") {
      cerr << "No game with name " << gameName << " found." << endl;
      cerr << "Supported games are: asterix, breakout, freeway, seaquest, space_invaders" << endl;
   }
   
   cerr << "---MinAtar-CPP---" << endl;
   if (initPy_) {
      cerr << "Initializing Python" << endl;
      Py_Initialize();
   }
   // Add the MinAtar path to the module search path
   CPyObject sysName = PyUnicode_FromString("sys");
   CPyObject sysMod = PyImport_Import(sysName);

   CPyObject version = PyObject_GetAttrString(sysMod, "version");
   CPyObject versionBytes = PyUnicode_AsEncodedString(version.getObject(), "UTF-8", "strict");
   string versionStr(PyBytes_AsString(versionBytes));
   cerr << "Python version: " << versionStr << endl;
   
   CPyObject sysPath = PyObject_GetAttrString(sysMod, "path");
   CPyObject sysPathAppend = PyObject_GetAttrString(sysPath, "append");
   CPyObject sysPathAppendArgs = Py_BuildValue("(s)", MINATAR_PATH_);
   PyObject_CallObject(sysPathAppend, sysPathAppendArgs);

   // Import the environment module
   CPyObject envName = PyUnicode_FromString("minatar.environment");
   cerr << "Importing MinAtar environment module" << endl;
   CPyObject envMod = PyImport_Import(envName);
   if (!envMod.is()) {
      cerr << "Could not import the MinAtar environment module." << endl;
      cerr << "Tried to import minatar.environment from path " << MINATAR_PATH_ << endl;
      cerr << "To change the path, compile with -DMINATAR_PATH=\"<path>\"" << endl;
      PyErr_Print();
      exit(1);
   }

   // Initialize the environment object
   cerr << "Creating MinAtar Environment: " << gameName << endl;
   cerr << "The given seed is: ";
   if (randomSeed == 0) {
      cerr << "Default Seed";
   } else {
      cerr << randomSeed;
   }
   cerr << endl;
   CPyObject envClass = PyObject_GetAttrString(envMod, "Environment");
   CPyObject pySeed = (randomSeed == 0 ? Py_None : PyLong_FromLong(randomSeed));
   CPyObject args = Py_BuildValue("(sfOO)",
				  gameName_.c_str(),
				  stickyAction,
				  (difficultyRamping ? Py_True : Py_False),
				  pySeed.getObject());
   env_ = PyObject_CallObject(envClass, args);

   // Get the shape 
   CPyObject shape = PyObject_CallMethod(env_, "state_shape", nullptr);
   PyObject* pyNumRows = PyList_GetItem(shape, 0);
   numRows_ = PyLong_AsLong(pyNumRows);
   PyObject* pyNumCols = PyList_GetItem(shape, 0);
   numCols_ = PyLong_AsLong(pyNumCols);
   PyObject* pyNumChannels = PyList_GetItem(shape, 2);
   numChannels_ = PyLong_AsLong(pyNumChannels);

   //Get the minimal action set
   CPyObject pyActSet = PyObject_CallMethod(env_, "minimal_action_set", nullptr);
   size_t numActs = PyList_Size(pyActSet);
   for (size_t i = 0; i < numActs; ++i) {
      PyObject* pyItem = PyList_GetItem(pyActSet, i);
      minActionSet_.push_back(PyLong_AsLong(pyItem));
   }

   //Resize the state
   curState_.resize(numRows_);
   for (size_t i = 0; i < numRows_; ++i) {
      curState_[i].resize(numCols_);
      for (size_t j = 0; j < numCols_; ++j) {
	 curState_[i][j].resize(numChannels_);
      }
   }
   curContinuousState_.resize(numChannels_);

   cerr << "Initializing the environment state" << endl;
   updateState();
   cerr << "---" << endl;
}

MinAtarInterface::~MinAtarInterface() {
   env_.release();
   if (finalizePy_) {
      cerr << "---MinAtar-CPP: Finalizing Python---" << endl;
      Py_Finalize();
   }
}

void MinAtarInterface::updateState() {
   CPyObject pyState = PyObject_CallMethod(env_, "state", nullptr);
   for (Py_ssize_t i = 0; i < Py_ssize_t(numRows_); ++i) {
      for (Py_ssize_t j = 0; j < Py_ssize_t(numCols_); ++j) {
	 for (Py_ssize_t k = 0; k < Py_ssize_t(numChannels_); ++k) {
	    CPyObject indexTuple = Py_BuildValue("(nnn)", i, j, k);
	    CPyObject pyEntry = PyObject_CallMethod(pyState, "item", "(O)", indexTuple.getObject());
	    curState_[i][j][k] = PyLong_AsLong(pyEntry);
	 }
      }
   }

   CPyObject pyCState = PyObject_CallMethod(env_, "continuous_state", nullptr);
   for (Py_ssize_t i = 0; i < Py_ssize_t(numChannels_); ++i) {
      curContinuousState_[i].clear();
      PyObject* objList = PyList_GetItem(pyCState, i);
      for (Py_ssize_t j = 0; j < PyList_Size(objList); ++j) {
	 PyObject* posTuple = PyList_GetItem(objList, j);
	 PyObject* pyX = PyTuple_GetItem(posTuple, 0);
	 double x = PyFloat_AsDouble(pyX);
	 PyObject* pyY = PyTuple_GetItem(posTuple, 1);
	 double y = PyFloat_AsDouble(pyY);
	 
	 curContinuousState_[i].push_back({x, y});
      }
   }
}

void MinAtarInterface::reset() {
   CPyObject result = PyObject_CallMethod(env_, "reset", nullptr);
   updateState();
}

void MinAtarInterface::act(unsigned long a, float& reward, bool& isGameOver)
{
   CPyObject pyResult = PyObject_CallMethod(env_, "act", "(k)", a);
   PyObject* pyReward = PyTuple_GetItem(pyResult, 0);
   reward = PyFloat_AsDouble(pyReward);
   PyObject* pyTerminal = PyTuple_GetItem(pyResult, 1);
   isGameOver = PyObject_IsTrue(pyTerminal);
   updateState();
}

const vector<vector<vector<size_t> > >& MinAtarInterface::state() const {
   return curState_;
}

const vector<vector<tuple<double, double> > >& MinAtarInterface::continuous_state() const {
   return curContinuousState_;
}

tuple<size_t, size_t, size_t> MinAtarInterface::state_shape() const {
   return make_tuple(numRows_, numCols_, numChannels_);
}

string MinAtarInterface::game_name() const {
   return gameName_;
}

size_t MinAtarInterface::num_actions() const {
   return 6;
}

const vector<size_t>& MinAtarInterface::minimal_action_set() const {
   return minActionSet_;
}

void MinAtarInterface::display_state(unsigned long time) {
   PyObject_CallMethod(env_, "display_state", "(k)", time);
}

void MinAtarInterface::close_display() {
   PyObject_CallMethod(env_, "close_display", nullptr);
}

MinAtarInterface::State MinAtarInterface::saveState() {
   CPyObject pyStr = PyObject_CallMethod(env_, "save_state", nullptr);
   CPyObject pyBytes = PyUnicode_AsEncodedString(pyStr.getObject(), "UTF-8", "strict");
   string stateStr(PyBytes_AsString(pyBytes));
   return {stateStr};
}

void MinAtarInterface::loadState(const State& state) {
   PyObject_CallMethod(env_, "load_state", "(s)", state.stateStr_.c_str());
}
