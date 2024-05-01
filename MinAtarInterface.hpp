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
   std::size_t numRows_;
   std::size_t numCols_;
   std::size_t numChannels_;
   std::vector<std::size_t> minActionSet_;
   std::vector<std::vector<std::vector<std::size_t> > > curState_;
   std::vector<std::vector<std::tuple<double, double> > > curContinuousState_;

   void updateState();
   
  public:
   class State {
      std::string stateStr_;
      State(std::string stateStr) : stateStr_{stateStr} {}
      friend class MinAtarInterface;
   };  
   
   MinAtarInterface(std::string gameName, float stickyAction=0.1, bool difficultyRamping=true, std::size_t randomSeed=0, bool initPy=true, bool finalizePy=true);
   ~MinAtarInterface();

   void reset();
   void act(unsigned long a, float& reward, bool& isGameOver);
   const std::vector<std::vector<std::vector<std::size_t> > >& state() const;
   const std::vector<std::vector<std::tuple<double, double> > >& continuous_state() const;

   std::tuple<std::size_t, std::size_t, std::size_t> state_shape() const;
   std::string game_name() const;
   std::size_t num_actions() const;
   const std::vector<std::size_t>& minimal_action_set() const;

   void display_state(unsigned long time=50);
   void close_display();

   State saveState();
   void loadState(const State& state);
};

#endif
