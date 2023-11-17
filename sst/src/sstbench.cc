// Copyright 2009-2023 NTESS. Under the terms
// of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2023, NTESS
// All rights reserved.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

#include <sst/core/sst_config.h>
#include "sstbench.h"

static auto CLOCK = "1GHz";

SumElements::SumElements(SST::ComponentId_t id, SST::Params &params) : SST::Component(id), repeats(0)
{

	output.init("SumElements-" + getName() + "-> ", 1, 0, SST::Output::STDOUT);

	maxRepeats = params.find<SST::Cycle_t>("repeats", 10);

	output.verbose(CALL_INFO, 1, 0, "Config: maxRepeats=%" PRIu64 "\n",
				   static_cast<uint64_t>(maxRepeats));

	// Just register a plain clock for this simple example

	this->output_link.link = configureLink("output_link", new SST::Event::Handler<SumElements>(this, &SumElements::response_handler));
	sst_assert(this->output_link.link, CALL_INFO, -1, "Error in %s: Failed to setup output link!", getName().c_str());
	this->output_link.capacity = params.find<uint32_t>("capacity", 10);

	this->input_a = configureLink("inputA", new SST::Event::Handler<SumElements>(this, &SumElements::inputAhandler));
	sst_assert(this->input_a, CALL_INFO, -1, "Error in %s: Link configuration failed A\n", getName().c_str());
	this->input_b = configureLink("inputB", new SST::Event::Handler<SumElements>(this, &SumElements::inputBHandler));
	sst_assert(this->input_b, CALL_INFO, -1, "Error in %s: Link configuration failed B\n", getName().c_str());

	// Tell SST to wait until we authorize it to exit
	registerAsPrimaryComponent();
	primaryComponentDoNotEndSim();
}

SumElements::~SumElements()
{
}

void SumElements::setup()
{
	output.verbose(CALL_INFO, 1, 0, "Component is being setup.\n");
}

void SumElements::finish()
{
	output.verbose(CALL_INFO, 1, 0, "Component is being finished.\n");
}

SignalGenerator::SignalGenerator(SST::ComponentId_t id, SST::Params &params) : SST::Component(id), repeats(0)
{

	output.init("SignalGenerator-" + getName() + "-> ", 1, 0, SST::Output::STDOUT);

	maxRepeats = params.find<SST::Cycle_t>("repeats", 10);

	output.output(CALL_INFO, "Config: maxRepeats=%" PRIu64 "\n",
				  static_cast<uint64_t>(maxRepeats));

	// Just register a plain clock for this simple example
	registerClock(CLOCK, new SST::Clock::Handler<SignalGenerator>(this, &SignalGenerator::clockTick));

	this->output_link.link = configureLink("output_link", new SST::Event::Handler<SignalGenerator>(this, &SignalGenerator::response_handler));
	this->output_link.capacity = params.find<uint32_t>("capacity", 10);
	sst_assert(this->output_link.link, CALL_INFO, -1, "Error in %s: Failed to setup output link!", getName().c_str());

	// Tell SST to wait until we authorize it to exit
	registerAsPrimaryComponent();
	primaryComponentDoNotEndSim();
}

SignalGenerator::~SignalGenerator()
{
}

void SignalGenerator::setup()
{
	output.verbose(CALL_INFO, 1, 0, "Signal Generator is being setup.\n");
}

void SignalGenerator::finish()
{
	output.verbose(CALL_INFO, 1, 0, "Signal Generator is being finished.\n");
}

bool SignalGenerator::clockTick(SST::Cycle_t currentCycle)
{
	// output.verbose(CALL_INFO, 1, 0, "Trying to Generate Signal.\n");
	if (output_link.is_full())
	{
		return false;
	}

	repeats++;
	auto event = new SimData;

	if (repeats == maxRepeats)
	{
		event->data = INT32_MAX;
		output_link.send(event);
		primaryComponentOKToEndSim();
		return true;
	}
	else
	{
		event->data = iter++;
		output_link.send(event);
		return false;
	}
}

Checker::Checker(SST::ComponentId_t id, SST::Params &params) : SST::Component(id)
{

	output.init("Checker-" + getName() + "-> ", 1, 0, SST::Output::STDOUT);

	this->input_link = configureLink("input_link", new SST::Event::Handler<Checker>(this, &Checker::handleEvent));
	sst_assert(this->input_link, CALL_INFO, -1, "Error in %s: Failed to setup input link!", getName().c_str());
	maxRepeats = params.find<SST::Cycle_t>("repeats", 10);
	// Tell SST to wait until we authorize it to exit
	registerAsPrimaryComponent();
	primaryComponentDoNotEndSim();
}

Checker::~Checker()
{
}

void Checker::setup()
{
	output.verbose(CALL_INFO, 1, 0, "Checker Generator is being setup.\n");
}

void Checker::finish()
{
	output.verbose(CALL_INFO, 1, 0, "Checker Generator is being finished.\n");
}
