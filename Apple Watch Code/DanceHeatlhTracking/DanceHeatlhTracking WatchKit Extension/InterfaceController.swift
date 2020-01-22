//
//  InterfaceController.swift
//  DanceHeatlhTracking WatchKit Extension
//
//  Created by Honza on 06/01/2020.
//  Copyright © 2020 Team32. All rights reserved.
//

import WatchKit
import Foundation
import CoreMotion
import HealthKit
import CoreML


class InterfaceController: WKInterfaceController {
    
// MARK: - Declaration of variables + actions
    @IBOutlet weak var timer: WKInterfaceTimer!
    @IBOutlet weak var yGravityLabel: WKInterfaceLabel!
    @IBOutlet weak var xRotationLabel: WKInterfaceLabel!
    @IBOutlet weak var spinsLabel: WKInterfaceLabel!
    @IBOutlet weak var startDancingButton: WKInterfaceButton!
    
    let healthStore = HKHealthStore()
    let motion = CMMotionManager()
    //let orientation = CMAttitude()
    var session: HKWorkoutSession!
    var builder: HKLiveWorkoutBuilder!
    var configuration: HKWorkoutConfiguration!
    
    var currentState : Int = 0
    var noOfSpinsCompleted : Int = 0
    var isAutorotating : Bool = true
    
    @IBAction func startDancingButtonClick() {
        if(currentState == 0){
            startSession()
        }
        else{
            endSession()
        }
    }
    
    // MARK: - Default Interface Setup
    override func awake(withContext context: Any?) {
        super.awake(withContext: context)
        // Configure interface objects here.
        authenticator()
        setUpEnv()
    }
    
    override func willActivate() {
        // This method is called when watch view controller is about to be visible to user
        super.willActivate()
    }
    
    override func didDeactivate() {
        // This method is called when watch view controller is no longer visible
        super.didDeactivate()
    }
    
// MARK: - Session Management
    func startSession(){
        print("Start")
        createHKWorkoutSession()
        currentState = 1
        startDancingButton.setTitle("Stop Dancing")
        motionData()
        session.startActivity(with: Date())
        builder.beginCollection(withStart: Date()) { (success, error) in
            self.setDurationTimerDate(.running)
        }
    }
        
    func endSession(){
        session.end()
        self.setDurationTimerDate(.ended)
        endCollectionOfWorkout()
        currentState = 0
        print("Stop")
        motionData()
        startDancingButton.setTitle("Start Dancing")
    }
    
    //Used from SpeedySloth example code
     func setDurationTimerDate(_ sessionState: HKWorkoutSessionState) {
         /// Obtain the elapsed time from the workout builder.
         /// - Tag: ObtainElapsedTime
         let timerDate = Date(timeInterval: -self.builder.elapsedTime, since: Date())
         
         // Dispatch to main, because we are updating the interface.
         DispatchQueue.main.async {
             self.timer.setDate(timerDate)
         }
         
         // Dispatch to main, because we are updating the interface.
         DispatchQueue.main.async {
             /// Update the timer based on the state we are in.
             /// - Tag: UpdateTimer
             sessionState == .running ? self.timer.start() : self.timer.stop()
         }
     }
    
// MARK: - HealthKit Management
    
    //https://developer.apple.com/documentation/healthkit/setting_up_healthkit
    
    func authenticator(){
        // The quantity type to write to the health store.
        let typesToShare: Set = [
            //HKQuantityType.workoutType()
            HKQuantityType.quantityType(forIdentifier: .heartRate)!,
            HKQuantityType.quantityType(forIdentifier: .activeEnergyBurned)!,
            HKQuantityType.quantityType(forIdentifier: .distanceWalkingRunning)!,
            HKQuantityType.quantityType(forIdentifier: .stepCount)!
        ]

        // The quantity types to read from the health store.
        let typesToRead: Set = [
            HKQuantityType.quantityType(forIdentifier: .heartRate)!,
            HKQuantityType.quantityType(forIdentifier: .activeEnergyBurned)!,
            HKQuantityType.quantityType(forIdentifier: .distanceWalkingRunning)!,
            HKQuantityType.quantityType(forIdentifier: .stepCount)!
        ]

        // Request authorization for those quantity types.
        healthStore.requestAuthorization(toShare: typesToShare, read: typesToRead) { (success, error) in
            // Handle error. No error handling in this sample project.
        }
    }
    
    func setUpEnv(){
        configuration = HKWorkoutConfiguration()
        configuration.activityType = .dance
        configuration.locationType = .indoor
    }
    
    func createHKWorkoutSession(){
        do {
            session = try HKWorkoutSession(healthStore: healthStore, configuration: configuration)
            builder = session.associatedWorkoutBuilder()
        } catch {
            dismiss()
            return
        }
        
        builder.dataSource = HKLiveWorkoutDataSource(healthStore: healthStore,
        workoutConfiguration: configuration)
    }
    
    func endCollectionOfWorkout(){
        builder.endCollection(withEnd: Date()) { (success, error) in
            self.builder.finishWorkout { (workout, error) in
                // Dispatch to main, because we are updating the interface.
                DispatchQueue.main.async() {
                    self.dismiss()
                }
            }
        }
    }
    
    
    // MARK: - Motion Manager

    //https://stackoverflow.com/questions/46756274/how-to-work-with-accelerometer-data-from-the-apple-watch
    
    func motionData(){
        if(self.currentState == 1){
            motion.deviceMotionUpdateInterval = 0.3

            
            motion.startDeviceMotionUpdates(to: OperationQueue.current!) { (data, error) in
                if let myData = data {
                    self.yGravityLabel.setText("y (gravity): " + String(format: "%.03f", myData.gravity.y))
                    self.xRotationLabel.setText("x (rotation): " + String(format: "%.03f", myData.rotationRate.x))

                     
                    print("x: \(myData.userAcceleration.x) y: \(myData.userAcceleration.y) z: \(myData.userAcceleration.z) rotRateX: \(myData.rotationRate.x) rotRateY: \(myData.rotationRate.y) rotRateZ: \(myData.rotationRate.z)")
                    
                    if(abs(myData.rotationRate.x) > 7 && abs(myData.gravity.y) < 0.2){
                        self.noOfSpinsCompleted += 1
                    }
                    self.spinsLabel.setText("Spins: \(self.noOfSpinsCompleted)")
                }
            }
        }
        else{
            self.spinsLabel.setText("Spins: \(self.noOfSpinsCompleted)")
            self.noOfSpinsCompleted = 0
            self.yGravityLabel.setText("y (gravity): 0")
            self.xRotationLabel.setText("x (rotation): 0")
            motion.stopDeviceMotionUpdates()
            return
        }
    }
}


    // MARK: - Creating URL Requests
    //https://www.youtube.com/watch?v=BFaZaUTF6m4

func getURLRequests(){
    let session = URLSession(configuration: .ephemeral)
    
    var url = URL(string: "http://localhost:3000/posts")
    var request = URLRequest(url : url!)

    request.cachePolicy = .reloadIgnoringLocalAndRemoteCacheData
    request.networkServiceType = .background
    
    //Part 3
    request.allowsCellularAccess = false
    
    //Part 4
    let taskWithRequest = session.dataTask(with: request)
    
    //Part 5
    taskWithRequest.currentRequest?.httpMethod
    
    //Part 6
    request.httpMethod = "POST"
    
    //Part 7
    request.addValue("application/json", forHTTPHeaderField: "content-type")
    
    //Part 8
    struct Post: Codable {
        //let id: Int
        let author: String
        let title: String
    }
    
    let encoder = JSONEncoder()
    let post = Post(author: "Example Author", title: "Example Title")
    do {
        let data = try encoder.encode(post)
        request.httpBody = data
    } catch let encoderError as NSError {
        //print("Encoder error: \(encodeError.localizedError.localizedDescription)")
    }
    
    //Part 9
    taskWithRequest.currentRequest?.httpMethod
    
    //Part 10
    let postTask = session.dataTask(with: request){ data, response, error in
        defer{}
        guard let data = data, let response = response as? HTTPURLResponse, response.statusCode == 201 else{
            print("No data or status code not created")
            return
        }
    
    
        let decoder = JSONDecoder()
        do {
            let post = try decoder.decode(Post.self, from: data)
            post
        } catch let decodeError as NSError {
            //Print
            return
        }
    }
    
    //Part 11
    postTask.currentRequest?.httpMethod
    postTask.currentRequest?.allHTTPHeaderFields
    postTask.currentRequest?.httpBody

    //Part 12
    postTask.resume()
    
    //Part 13
    postTask.currentRequest?.httpBody
}


