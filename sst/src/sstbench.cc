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

SumWithFibonacci::SumWithFibonacci(SST::ComponentId_t id, SST::Params &params) : SST::Component(id), repeats(0)
{

	maxRepeats = params.find<SST::Cycle_t>("repeats", 10);
	// output.output(CALL_INFO, "Config: maxRepeats=%" PRIu64 "\n",
	// 		static_cast<uint64_t>(maxRepeats));

	this->output_link.link = configureLink("output_link", new SST::Event::Handler<SumWithFibonacci>(this, &SumWithFibonacci::response_handler));
	sst_assert(this->output_link.link, CALL_INFO, -1, "Error in %s: Failed to setup output link!", getName().c_str());
	this->output_link.capacity = params.find<uint32_t>("capacity", 10);

	this->input_a = configureLink("inputA", new SST::Event::Handler<SumWithFibonacci>(this, &SumWithFibonacci::inputAhandler));
	sst_assert(this->input_a, CALL_INFO, -1, "Error in %s: Link configuration failed A\n", getName().c_str());
	this->input_b = configureLink("inputB", new SST::Event::Handler<SumWithFibonacci>(this, &SumWithFibonacci::inputBHandler));
	sst_assert(this->input_b, CALL_INFO, -1, "Error in %s: Link configuration failed B\n", getName().c_str());

	this->fib_repeats = params.find<uint64_t>("fib", 1);

	// Tell SST to wait until we authorize it to exit
	registerAsPrimaryComponent();
	primaryComponentDoNotEndSim();
}

SumWithFibonacci::~SumWithFibonacci()
{
}

void SumWithFibonacci::setup() {}

void SumWithFibonacci::finish() {}

SignalGenerator::SignalGenerator(SST::ComponentId_t id, SST::Params &params) : SST::Component(id), repeats(0)
{

	output.init("SignalGenerator-" + getName() + "-> ", 1, 0, SST::Output::STDOUT);

	maxRepeats = params.find<SST::Cycle_t>("repeats", 10);

	// output.output(CALL_INFO, "Config: maxRepeats=%" PRIu64 "\n",
	// 			  static_cast<uint64_t>(maxRepeats));

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
}

void SignalGenerator::finish()
{
}

bool SignalGenerator::clockTick(SST::Cycle_t currentCycle)
{
	if (output_link.is_full())
	{
		return false;
	}

	repeats++;
	auto event = new SimData;

	event->data = iter++;
	output_link.send(event);

	if (repeats == maxRepeats) {
		primaryComponentOKToEndSim();
		return true;
	} else {
		return false;
	}
}

Checker::Checker(SST::ComponentId_t id, SST::Params &params) : SST::Component(id), repeats(0)
{

	output.init("Checker-" + getName() + "-> ", 1, 0, SST::Output::STDOUT);

	this->input_link = configureLink("input_link", new SST::Event::Handler<Checker>(this, &Checker::handleEvent));
	sst_assert(this->input_link, CALL_INFO, -1, "Error in %s: Failed to setup input link!", getName().c_str());
	maxRepeats = params.find<SST::Cycle_t>("repeats", 10);
	// output.output(CALL_INFO, "Config: maxRepeats=%" PRIu64 "\n",
	// 		static_cast<uint64_t>(maxRepeats));
	// Tell SST to wait until we authorize it to exit
	registerAsPrimaryComponent();
	primaryComponentDoNotEndSim();
}

Checker::~Checker()
{
}

void Checker::setup()
{
}

void Checker::finish()
{
}

SimpleExternalElement::SimpleExternalElement( SST::ComponentId_t id, SST::Params& params ) :
	SST::Component(id), repeats(0) {

	output.init("sstbench-" + getName() + "-> ", 1, 0, SST::Output::STDOUT);

	printFreq  = params.find<SST::Cycle_t>("printFrequency", 5);
	maxRepeats = params.find<SST::Cycle_t>("repeats", 10);

	if( ! (printFreq > 0) ) {
		output.fatal(CALL_INFO, -1, "Error: printFrequency must be greater than zero.\n");
	}

	output.verbose(CALL_INFO, 1, 0, "Config: maxRepeats=%" PRIu64 ", printFreq=%" PRIu64 "\n",
		static_cast<uint64_t>(maxRepeats), static_cast<uint64_t>(printFreq));

	// Just register a plain clock for this simple example
    	registerClock("100MHz", new SST::Clock::Handler<SimpleExternalElement>(this, &SimpleExternalElement::clockTick));

	// Tell SST to wait until we authorize it to exit
    	registerAsPrimaryComponent();
    	primaryComponentDoNotEndSim();
}

SimpleExternalElement::~SimpleExternalElement() {

}

void SimpleExternalElement::setup() {
	output.verbose(CALL_INFO, 1, 0, "Component is being setup.\n");
}

void SimpleExternalElement::finish() {
	output.verbose(CALL_INFO, 1, 0, "Component is being finished.\n");
}

bool SimpleExternalElement::clockTick( SST::Cycle_t currentCycle ) {

	if( currentCycle % printFreq == 0 ) {
		output.verbose(CALL_INFO, 1, 0, "Hello World!\n");
	}

	repeats++;

	if( repeats == maxRepeats ) {
		primaryComponentOKToEndSim();
		return true;
	} else {
		return false;
	}
}
