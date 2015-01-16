'use strict';

/* Controllers */

var cpuVisorControllers = angular.module('cpuVisorControllers', ['cpuVisorDirectives']);

cpuVisorControllers.controller('LandingCtrl', ['$scope', '$location', 'StartQuery',
  function ($scope, $location, StartQuery) {

    $scope.query_text = '';

    $scope.search = function(query_text) {
      if (query_text) {
        StartQuery.execute({}, function(queryIfo) {
          $location.path('/results/' + queryIfo.query_id + '/' + btoa(query_text));
        });
      }
    };

  }]);

cpuVisorControllers.controller('RankingCtrl', ['$scope', '$routeParams', '$location', '$http', '$timeout', 'socketFactory',
  'StartQuery', 'AddTrs', 'TrainModel', 'Rank', 'Ranking',
  function ($scope, $routeParams, $location, $http, $timeout, socketFactory, StartQuery, AddTrs, TrainModel, Rank, Ranking) {

    $scope.StateEnum = {
      QS_DATACOLL: 0,
      QS_DATACOLL_COMPLETE: 1,
      QS_TRAINING: 2,
      QS_TRAINED: 3,
      QS_RANKING: 4,
      QS_RANKED: 5
    };

    $scope.startTraining = function() {
      TrainModel.execute({qid: $scope.query_id});
    };

    // Start -----

    $scope.query = atob($routeParams.queryText);
    $scope.query_id = $routeParams.queryId;
    $scope.pages_retrieved = 0;
    $scope.ranking = {'rlist': []};
    $scope.state = $scope.StateEnum.QS_DATACOLL;

    var io_namespace = '/api/query/notifications';
    console.log('Creating socket with namespace: ' + io_namespace);
    $scope.notification_socket = socketFactory({
      ioSocket: io.connect(io_namespace)
    });

    $scope.notification_socket.on('notification', function(notify_data) {
      if (notify_data.id === $scope.query_id) {
        switch (notify_data.type) {
          case 'NTFY_STATE_CHANGE':
            $scope.state = $scope.StateEnum[notify_data.data];

            console.log('State changed to: ' + $scope.state);
            switch ($scope.state) {
              case $scope.StateEnum.QS_DATACOLL_COMPLETE:
                $scope.startTraining();
                break;
              case $scope.StateEnum.QS_TRAINED:
                Rank.execute({qid: $scope.query_id});
                break;
              case $scope.StateEnum.QS_RANKED:
                Ranking.get({qid: $scope.query_id}, function(ranking_obj) {
                  //console.log(ranking);
                  $scope.ranking = ranking_obj.ranking;
                  console.log($scope.ranking);
                  console.log('Ranking is now of length: ' + $scope.ranking.rlist.length);
                });
                break;
            }
            break;
          case 'NTFY_IMAGE_PROCESSED':
            console.log('Image processed: ' + notify_data.data);
            break;
          case 'NTFY_ALL_IMAGES_PROCESSED':
            console.log('All images processed!');
            break;
        }
      }
    });

    AddTrs.execute({qid: $scope.query_id, q: $scope.query});

    // timeout for image addition
    $timeout(function() {
      if ($scope.state <= $scope.StateEnum.QS_DATACOLL_COMPLETE) {
        $scope.startTraining()
      }
    }, 20000.0);

    $scope.getImgSrc = function(ritem) {
      return 'dsetimages/' + ritem.path;
    };

  }]);
