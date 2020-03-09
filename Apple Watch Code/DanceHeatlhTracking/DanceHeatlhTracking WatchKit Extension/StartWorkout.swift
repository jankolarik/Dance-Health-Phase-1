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
    
    override func didAppear() {
        super.didAppear()
        nextButton.setEnabled(enableButton)
    }
    
    override func contextForSegue(withIdentifier segueIdentifier: String) -> Any? {
        return id
    }

}
