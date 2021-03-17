#ifndef MINATAR_INTERFACE_HPP
#define MINATAR_INTERFACE_HPP

#include "CPyObject.hpp"
#include <string>
#include <tuple>
#include <vector>

class MinAtarInterface {
  private:
   // Should the object initialize/finalize the Python interpreter?
   // Yes if it's the only thing using the Python interpreter (default)
   // No if something else will intialize/finalize the interpreter
   bool initPy_;
   bool finalizePy_;

   CPyObject env_;
   
   std::string gameName_;
   size_t numRows_;
   size_t numCols_;
   size_t numChannels_;
   std::vector<size_t> minActionSet_;
   std::vector<std::vector<std::vector<size_t> > > curState_;

   void updateState();
   
  public:
   class State {
      std::string stateStr_;
      State(std::string stateStr) : stateStr_{stateStr} {}
      friend class MinAtarInterface;
   };  
   
   MinAtarInterface(std::string gameName, float stickyAction=0.1, bool difficultyRamping=true, size_t randomSeed=0, bool initPy=true, bool finalizePy=true);
   ~MinAtarInterface();

   void reset();
   void act(unsigned long a, float& reward, bool& isGameOver);
   const std::vector<std::vector<std::vector<size_t> > >& state() const;

   std::tuple<size_t, size_t, size_t> state_shape() const;
   std::string game_name() const;
   size_t num_actions() const;
   const std::vector<size_t>& minimal_action_set() const;

   void display_state(unsigned long time=50);
   void close_display();

   State saveState();
   void loadState(const State& state);
};

#endif
