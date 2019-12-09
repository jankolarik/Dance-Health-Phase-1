//
//  startDanceView.swift
//  DanceHealthTrack WatchKit Extension
//
//  Created by Honza on 07/12/2019.
//  Copyright © 2019 Team32. All rights reserved.
//

import WatchKit
import Foundation
import HealthKit
import CoreML
import CoreMotion


class startDanceView: WKInterfaceController {

    
    
    let healthStore = HKHealthStore()
    let motion = CMMotionManager()
    var session: HKWorkoutSession!
    var builder: HKLiveWorkoutBuilder!
    var configuration: HKWorkoutConfiguration!
    var motionManager = CMMotionManager()

    @IBOutlet weak var timer: WKInterfaceTimer!
    
    @IBAction func startWorkoutButton() {
        print("START")
        startSession()
        //startAccelerometers()
        testData()
        //workoutBuilder(builder, didCollectDataOf: collectedTypes)
    }

    @IBAction func stopWorkoutButton() {
        print("STOP")
        endSession()
    }
    
    
    // MARK: - Basic screen loading
    override func didAppear() {
        super.didAppear()
        authenticator()
        setUpEnv()
        createHKWorkoutSession()
    }
    
    override func awake(withContext context: Any?) {
        super.awake(withContext: context)
        // Configure interface objects here.
    }
    
    override func willActivate() {
        // This method is called when watch view controller is about to be visible to user
        super.willActivate()
    }
    
    override func didDeactivate() {
        // This method is called when watch view controller is no longer visible
        super.didDeactivate()
    }
    
    // MARK: - Workout Functionality
    func workoutBuilder(_ workoutBuilder: HKLiveWorkoutBuilder, didCollectDataOf collectedTypes: Set<HKSampleType>) {
        for type in collectedTypes {
            guard let quantityType = type as? HKQuantityType else {
                return // Nothing to do.
            }
            
            // Calculate statistics for the type.
            let statistics = workoutBuilder.statistics(for: quantityType)
            print(quantityType)
            
            do{
                let heartRateUnit = HKUnit.count().unitDivided(by: HKUnit.minute())
                let value = statistics!.mostRecentQuantity()?.doubleValue(for: heartRateUnit)
                let roundedValue = Double( round( 1 * value! ) / 1 )
                print("\(roundedValue) BPM")
            }
            catch{
            
            }
            
            DispatchQueue.main.async() {
                // Update the user interface.
            }
        }
    }
    

    
    func authenticator(){
        // The quantity type to write to the health store.
        let typesToShare: Set = [
            HKQuantityType.workoutType()
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
    
    func startSession(){
        session.startActivity(with: Date())
        builder.beginCollection(withStart: Date()) { (success, error) in
            self.setDurationTimerDate(.running)
        }
    }
    
    func endSession(){
        session.end()
        self.setDurationTimerDate(.ended)
        builder.endCollection(withEnd: Date()) { (success, error) in
            self.builder.finishWorkout { (workout, error) in
                // Dispatch to main, because we are updating the interface.
                DispatchQueue.main.async() {
                    self.dismiss()
                }
            }
        }
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
    
    
    // MARK: - Motion Tracking & ML

//https://developer.apple.com/documentation/coremotion/getting_raw_accelerometer_events
    func startAccelerometers() {
       // Make sure the accelerometer hardware is available.
       if self.motion.isAccelerometerAvailable {
          self.motion.accelerometerUpdateInterval = 1.0 / 60.0  // 60 Hz
          self.motion.startAccelerometerUpdates()

          // Configure a timer to fetch the data.
          let timerOne = Timer(fire: Date(), interval: (1.0/60.0),
                repeats: true, block: { (timer) in
             // Get the accelerometer data.
             if let data = self.motion.accelerometerData {
                let x = data.acceleration.x
                let y = data.acceleration.y
                let z = data.acceleration.z
                print(x)
                print(y)
                print(z)

                // Use the accelerometer data in your app.
             }
          })

          // Add the timer to the current run loop.
          //RunLoop.current.add(self.timer!, forMode: .defaultRunLoopMode)
       }
    }
    
    //https://stackoverflow.com/questions/46756274/how-to-work-with-accelerometer-data-from-the-apple-watch
    func testData(){
        motionManager.accelerometerUpdateInterval = 0.2

        motionManager.startAccelerometerUpdates(to: OperationQueue.current!) { (data, error) in
            if let myData = data {

                 print("x: \(myData.acceleration.x) y: \(myData.acceleration.y) z: \(myData.acceleration.z)")
                if myData.acceleration.x > 1.5 && myData.acceleration.y > 1.5 {

                }
            }
        }
    }

}
