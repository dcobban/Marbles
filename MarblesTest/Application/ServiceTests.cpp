#include <application/event.h>
#include <application/service.h>
#include <application/application.h>

BOOST_AUTO_TEST_SUITE( service )

struct ExecutedService
{
	bool executed;
	~ExecutedService() {}
	ExecutedService() 
	{
		executed = false;
		
		marbles::shared_service active = marbles::service::active();
		active->post(std::bind(&ExecutedService::Execute, this));
	}

	void Execute() 
	{
		executed = true; 
		marbles::service::active()->stop();
	}
};

struct ApplicationStop
{
	int count;
	int end;
	marbles::event<void ()> update;
	marbles::task updateTask;

	ApplicationStop(const int count) 
		: count(0)
		, end(count)
	{
		updateTask = update += std::bind(&ApplicationStop::Update, this);
		update();
	}

	~ApplicationStop() 
	{
		update -= updateTask;
		update.clear();
	}

	void Update() 
	{
		++count;
		if (count == end)
		{
			marbles::application::get()->stop(0);
		}
		else
		{
			update();
		}
	}
};

BOOST_AUTO_TEST_CASE( single_thread_test )
{
	BOOST_MESSAGE( "service.single_thread_test" );

	const int numCyclesToStop = 1000;
	marbles::application app;
	marbles::shared_service executeTest = app.start<ExecutedService>();
	std::vector<marbles::shared_service> racers;
	racers.push_back(app.start<ApplicationStop>(numCyclesToStop));
	racers.push_back(app.start<ApplicationStop>(numCyclesToStop));
	racers.push_back(app.start<ApplicationStop>(numCyclesToStop));
	racers.push_back(app.start<ApplicationStop>(numCyclesToStop));
	racers.push_back(app.start<ApplicationStop>(numCyclesToStop));

	app.run(1);

	BOOST_CHECK(executeTest->provider<ExecutedService>()->executed);
	marbles::shared_service winner = *racers.begin();
	for(std::vector<marbles::shared_service>::iterator i = racers.begin() + 1;
		i != racers.end();
		++i)
	{
		if (winner->provider<ApplicationStop>()->count < (*i)->provider<ApplicationStop>()->count)
		{
			winner = *i;
		}
	}
	const int stopCount = winner->provider<ApplicationStop>()->count;
	BOOST_CHECK_EQUAL(numCyclesToStop, stopCount);
}

BOOST_AUTO_TEST_CASE( multiple_thread_test )
{
	BOOST_MESSAGE( "service.multiple_thread_test" );
	const int numCyclesToStop = 1000;
	marbles::application app;
	marbles::shared_service executeTest = app.start<ExecutedService>();
	std::vector<marbles::shared_service> racers;
	racers.push_back(app.start<ApplicationStop>(numCyclesToStop));
	racers.push_back(app.start<ApplicationStop>(numCyclesToStop));
	racers.push_back(app.start<ApplicationStop>(numCyclesToStop));
	racers.push_back(app.start<ApplicationStop>(numCyclesToStop));
	racers.push_back(app.start<ApplicationStop>(numCyclesToStop));

	app.run();

	BOOST_CHECK(executeTest->provider<ExecutedService>()->executed);
	marbles::shared_service winner = *racers.begin();
	for(std::vector<marbles::shared_service>::iterator i = racers.begin() + 1;
		i != racers.end();
		++i)
	{
		if (winner->provider<ApplicationStop>()->count < (*i)->provider<ApplicationStop>()->count)
		{
			winner = *i;
		}
	}
	const int stopCount = winner->provider<ApplicationStop>()->count;
	BOOST_CHECK_EQUAL(numCyclesToStop, stopCount);
}

BOOST_AUTO_TEST_SUITE_END()
