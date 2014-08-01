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
		active->post([this]() { this->Execute(); });
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
	int stop;
	marbles::event<> update;
	marbles::event<>::shared_handler updateHandler;

	ApplicationStop(const int end) 
	: count(0)
	, stop(end)
	{
		ApplicationStop* pThis = this;
		auto fn = [pThis]() { pThis->OnUpdate(); };
		auto handler = update += std::move(fn);
		updateHandler = handler.lock();
		update();
		//OnUpdate() ;
	}

	~ApplicationStop() 
	{
		update -= updateHandler;
		update.clear();
	}

	void OnUpdate() 
	{
		if (++count != stop)
		{
			update();
		}
		else
		{	// We have a winner stop the application
			marbles::application::get()->stop(0);
		}
	}
};

BOOST_AUTO_TEST_CASE( single_thread_test )
{
	BOOST_MESSAGE( "service.single_thread_test" );

	const int numCyclesToStop = 1000;
	marbles::application app;
	marbles::shared_service service = app.start<ExecutedService>();
	std::vector<marbles::shared_service> racers;
	racers.push_back(app.start<ApplicationStop>(numCyclesToStop));
	racers.push_back(app.start<ApplicationStop>(numCyclesToStop));
	racers.push_back(app.start<ApplicationStop>(numCyclesToStop));
	racers.push_back(app.start<ApplicationStop>(numCyclesToStop));
	racers.push_back(app.start<ApplicationStop>(numCyclesToStop));

	app.run(1);

	BOOST_CHECK(service->provider<ExecutedService>()->executed);
	marbles::shared_service winner = *racers.begin();
	for(auto i : racers)
	{
		if (winner->provider<ApplicationStop>()->count < i->provider<ApplicationStop>()->count)
		{
			winner = i;
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
	marbles::shared_service service = app.start<ExecutedService>();
	std::vector<marbles::shared_service> racers;
	racers.push_back(app.start<ApplicationStop>(numCyclesToStop));
	racers.push_back(app.start<ApplicationStop>(numCyclesToStop));
	racers.push_back(app.start<ApplicationStop>(numCyclesToStop));
	racers.push_back(app.start<ApplicationStop>(numCyclesToStop));
	racers.push_back(app.start<ApplicationStop>(numCyclesToStop));

	app.run();

	BOOST_CHECK(service->provider<ExecutedService>()->executed);
	marbles::shared_service winner = *racers.begin();
	for(auto i : racers)
	{
		if (winner->provider<ApplicationStop>()->count < i->provider<ApplicationStop>()->count)
		{
			winner = i;
		}
	}
	const int stopCount = winner->provider<ApplicationStop>()->count;
	BOOST_CHECK_EQUAL(numCyclesToStop, stopCount);
}

BOOST_AUTO_TEST_SUITE_END()
