'use strict';

/* App Module */

var cpuVisorApp = angular.module('cpuVisorApp', [
  'ngRoute',
  'ngAnimate',
  'btford.socket-io',
  'mgcrea.ngStrap',

  'cpuVisorControllers',
  'cpuVisorServices',
  'cpuVisorDirectives'
]);

cpuVisorApp.config(['$routeProvider',
  function ($routeProvider) {
    $routeProvider.
      when('/', {
        templateUrl: 'partials/landing.html',
        controller: 'LandingCtrl'
      }).
      when('/results/:queryText', {
        templateUrl: 'partials/ranking.html',
        controller: 'RankingCtrl'
      }).
      otherwise({
        redirectTo: '/'
      });
  }]);
