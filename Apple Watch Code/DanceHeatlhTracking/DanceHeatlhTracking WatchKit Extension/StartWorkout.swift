//
//  StartWorkout.swift
//  DanceHeatlhTracking WatchKit Extension
//
//  Created by Honza on 25/02/2020.
//  Copyright © 2020 Team32. All rights reserved.
//  Adapted from SpeedySloth example from Apple

import WatchKit
import Foundation
import HealthKit


class StartWorkout: WKInterfaceController {
    
    var enableButton = false
    var id = ""
    
    @IBOutlet weak var nextButton: WKInterfaceButton!
    @IBOutlet weak var idTextField: WKInterfaceTextField!
    
    @IBAction func idTextFieldAction(_ value: NSString?) {
        if(value != ""){
            id = value! as String
            enableButton = true
        }
    }

    /*let healthStore = HKHealthStore()
    
    //https://developer.apple.com/documentation/healthkit/setting_up_healthkit
    func authenticator(){
        // The quantity type to write to the health store.
        let typesToShare: Set = [
            HKQuantityType.workoutType()
        ]

        // The quantity types to read from the health store.
        let typesToRead: Set = [
            HKQuantityType.workoutType()
        ]

        // Request authorization for those quantity types.
        healthStore.requestAuthorization(toShare: typesToShare, read: typesToRead) { (success, error) in
            // Handle error. No error handling in this sample project.
        }
    }*/
    
    override func didAppear() {
        super.didAppear()
        nextButton.setEnabled(enableButton)
        //authenticator()
    }
    
    /*override func contextForSegueWithIdentifier(segueIdentifier: String) -> AnyObject? {
        // You may want to set the context's identifier in Interface Builder and check it here to make sure you're returning data at the proper times

        // Return data to be accessed in ResultsController
        return id as AnyObject
    }*/
    
    
    override func contextForSegue(withIdentifier segueIdentifier: String) -> Any? {
        /*if segueIdentifier == "startWorkout" {
            /// - Tag: WorkoutConfiguration
            let configuration = HKWorkoutConfiguration()
            configuration.activityType = .dance
            configuration.locationType = .outdoor
            
            return WorkoutSessionContext(healthStore: healthStore, configuration: configuration)
        }*/
        
        return id
    }

}
