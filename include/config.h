#ifndef CONFIG_H
#define CONFIG_H

#include "acceptance_rule.h"
#include "inter_operator.h"
#include "intra_operator.h"
#include "ruin_method.h"
#include "sorter.h"

#include <functional>
#include <memory>
#include <vector>

class Listener 
{
public:
    virtual ~Listener() = default;

    virtual void OnStart() = 0;
    virtual void OnUpdated(const SpecificSolution &solution, int objective) = 0;
    virtual void OnEnd(const SpecificSolution &solution, int objective) = 0;
};

struct Config 
{
    uint32_t random_seed; /**< The random seed for the optimization process. */
    double time_limit;    /**< The time limit (in seconds) for the optimization process. */
    std::unique_ptr<Listener> listener; /**< The listener for receiving optimization events. */
};

struct SpecificConfig : public Config 
{
    double blink_rate;    /**< The blink rate for the SplitReinsertion process. */
    std::vector<std::unique_ptr<InterOperator>>
        inter_operators; /**< The inter-operators for optimizing the solution. */
    std::vector<std::unique_ptr<IntraOperator>>
        intra_operators; /**< The intra-operators for optimizing the solution. */
    std::function<std::unique_ptr<AcceptanceRule>()>
        acceptance_rule; /**< The acceptance rule for accepting new solutions. */
    std::unique_ptr<RuinMethod>
        ruin_method;       /**< The ruin method for destroying parts of the solution. */
    Sorter sorter; /**< The sorter for sorting customers during the perturbation process. */
};

#endif