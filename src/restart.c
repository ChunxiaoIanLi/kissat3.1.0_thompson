#include "restart.h"
#include "backtrack.h"
#include "bump.h"
#include "decide.h"
#include "internal.h"
#include "kimits.h"
#include "logging.h"
#include "print.h"
#include "reluctant.h"
#include "report.h"

#include <inttypes.h>
#include <math.h>
#include <time.h>
#include "Thompson.hpp"
#include "ucb.h"

bool kissat_restarting (kissat *solver) {
  assert (solver->unassigned);
  if (!GET_OPTION (restart))
    return false;
  if (!solver->level)
    return false;
  if (CONFLICTS < solver->limits.restart.conflicts)
    return false;
  if (solver->stable)
    return kissat_reluctant_triggered (&solver->reluctant);
  const double fast = AVERAGE (fast_glue);
  const double slow = AVERAGE (slow_glue);
  const double margin = (100.0 + GET_OPTION (restartmargin)) / 100.0;
  const double limit = margin * slow;
  kissat_extremely_verbose (solver,
                            "restart glue limit %g = "
                            "%.02f * %g (slow glue) %c %g (fast glue)",
                            limit, margin, slow,
                            (limit > fast    ? '>'
                             : limit == fast ? '='
                                             : '<'),
                            fast);
  return (limit <= fast);
}

void kissat_update_focused_restart_limit (kissat *solver) {
  assert (!solver->stable);
  limits *limits = &solver->limits;
  uint64_t restarts = solver->statistics.restarts;
  uint64_t delta = GET_OPTION (restartint);
  if (restarts)
    delta += kissat_logn (restarts) - 1;
  limits->restart.conflicts = CONFLICTS + delta;
  kissat_extremely_verbose (solver,
                            "focused restart limit at %" PRIu64
                            " after %" PRIu64 " conflicts ",
                            limits->restart.conflicts, delta);
}

void kissat_restart (kissat *solver) {
  START (restart);
  INC (restarts);
  if (solver->stable)
    INC (stable_restarts);
  else
    INC (focused_restarts);
  unsigned level = 0;
  kissat_extremely_verbose (solver,
                            "restarting after %" PRIu64 " conflicts"
                            " (limit %" PRIu64 ")",
                            CONFLICTS, solver->limits.restart.conflicts);
  LOG ("restarting to level %u", level);
  kissat_backtrack_in_consistent_state (solver, level);

  if(level != 0){
    //printf("decisions: %d\n", solver->reset_decisions);
    //printf("conflicts: %d\n", solver->reset_conflicts);
    // Seed the random number generator
    srand(time(NULL));

    //Thompson

    //compute local learning rate
    double localLearningRate = (solver->reset_conflicts * 1.0) / solver->reset_decisions;
    solver-> reset_conflicts = 0;
    solver-> reset_decisions = 0;
    //printf("local learning rate: %0.2f\n", localLearningRate);
    //printf("learning rateEMB: %0.2f\n\n", solver->learningRateEMA);
    //update success and failures
    // if (solver-> resetTotalTimes != 0){
    //   if (localLearningRate >= solver->learningRateEMA){
    //       if (solver->resetPrevLever == 0){
    //         solver->reset_wins++;
    //       }
    //       else{
    //         solver->restart_wins++;
    //       }
    //   }
    //   else{
    //     if (solver->resetPrevLever == 0){
    //       solver->reset_loses++;
    //     }
    //     else{
    //       solver->restart_loses++;
    //     }
    //   }
    // }
    // //update LLR-MAB
    // solver-> learningRateEMA *= solver-> resetDecay;
    // solver->learningRateEMA += localLearningRate * (1.0 - solver->resetDecay);

    //create a struct record
    struct record pre_record = {solver->resetPrevLever, localLearningRate};
    //sw_ucb: add record to solver->window
    add_record(solver->window, pre_record);
    // printf("select level: %d with local learning rate: %0.2f\n\n", solver->resetPrevLever, localLearningRate);

    unsigned int reset_count = 0;
    unsigned int restart_count = 0;
    double x_sum_reset = 0;
    double x_sum_restart = 0;
    unsigned int w_size = 0; // number of records in solver->window
   // w_size = solver->window->counts if solver->window->counts < solver->window_sz else solver->window_sz;
    if (solver->window->counts < solver->window_sz){
      w_size = solver->window->counts;
    }
    else{
      w_size = solver->window_sz;
    }
    for (int i = 0; i < w_size; i++){
      if (solver->window->records[i].bandit == 0){
        reset_count++;
        x_sum_reset += solver->window->records[i].reward;
      }
      else{
        restart_count++;
        x_sum_restart += solver->window->records[i].reward;
      }
    }
    if (restart_count == 0){
      solver->resetPrevLever = 1;
    } 
    else if (reset_count == 0){
      solver->resetPrevLever = 0;
    }
    else{
      double x_bar_reset = x_sum_reset / reset_count;
      double x_bar_restart = x_sum_restart / restart_count;
      double exp_reset = sqrt(2 * log(w_size) / reset_count) * solver->ucb_c;
      double exp_restart = sqrt(2 * log(w_size) / restart_count) * solver->ucb_c;
      double ucb_reset = x_bar_reset + exp_reset;
      double ucb_restart = x_bar_restart + exp_restart;
      if (ucb_reset > ucb_restart){
        solver->resetPrevLever = 0;
      }
      else{
        solver->resetPrevLever = 1;
      }
      // printf("Reset: Q: %f, EXP: %f(%d/%d), UCB: %f\n", x_bar_reset, exp_reset, reset_count, w_size, ucb_reset);
      // printf("Restart: Q: %f, EXP: %f(%d/%d), UCB: %f\n", x_bar_restart, exp_restart, restart_count, w_size, ucb_restart);
    }
    // printf("reset_wins: %0.2f\n", solver->reset_wins);
    // printf("reset_loses: %0.2f\n", solver->reset_loses);
    // printf("restart_wins: %0.2f\n", solver->restart_wins);
    // printf("restart_loses: %0.2f\n", solver->restart_loses);
    //select a lever
    // solver->resetPrevLever = select_lever(solver->reset_wins, solver->reset_loses, solver->restart_wins, solver->restart_loses);
    // solver->resetTotalTimes++;
    // if (solver->resetPrevLever == 0){
    //   solver->reset_wins *= solver->resetDecay;
    //   solver->reset_loses *= solver->resetDecay;
    // }
    // else{
    //   solver->restart_wins *= solver->resetDecay;
    //   solver->restart_loses *= solver->resetDecay;
    // }


    if (solver->resetPrevLever == 0) {
      //reset activities
      for (all_variables(idx)) {
        kissat_update_heap(solver, &solver->scores, idx, (double) rand() / RAND_MAX*0.00001);
      }
      kissat_update_scores(solver);
      solver->nof_resets++;
    }
    else{
      solver->nof_restarts++; 
    }
  }
  else{
    solver->nof_restarts++;  
  }

  if (!solver->stable)
    kissat_update_focused_restart_limit (solver);


  REPORT (1, 'R');
  STOP (restart);
}
