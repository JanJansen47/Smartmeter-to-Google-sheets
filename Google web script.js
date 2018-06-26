 
// Steps are valid as of 2016 November 12th.
// 0) From Google spreadsheet, Tools &gt; Scriipt Editor...
// 1) Write your code
// 2) Save and give a meaningful name
// 3) Run and make sure "doGet" is selected
//    You can set a method from Run menu
// 4) When you run for the first time, it will ask 
//    for the permission. You must allow it.
//    Make sure everything is working as it should.
// 5) From Publish menu &gt; Deploy as Web App...
//    Select a new version everytime it's published
//    Type comments next to the version
//    Execute as: "Me (your email address)"
//    MUST: Select "Anyone, even anonymous" on "Who has access to this script"
//    For the first time it will give you some prompt(s), accept it.
//    You will need the given information (url) later. This doesn't change, ever!
 
// Saving the published URL helps for later.
// https://script.google.com/macros/s/---Your-Script-ID--Goes-Here---/exec
// https://script.google.com/macros/s/---Your-Script-ID--Goes-Here---/exec?tag=test&amp;value=-1
 
// This method will be called first or hits first  
function doGet(e){
  Logger.log("--- doGet ---");
 console.log ("do get started");
var Tbuit = "1", Tbinnen = "1",mEVLT = "2", mEVHT = "1",mEOLT = "2", mEOHT = "1", mEAT = "1",mEAV = "2", mGAS = "1", CRC2 = "0", CRC3 = "0";
     
  try {
 
     Tbinnen =   e.parameters.Tbinnen_val;
    Tbuit =     e.parameters.Tbuit_val;
    mEVLT =     e.parameters.mEVLT_val;
    mEVHT =     e.parameters.mEVHT_val;
    mEOLT = e.parameters.mEOLT_val;
    mEOHT = e.parameters.mEOHT_val;
    mEAT =  e.parameters.mEAT_val;
    mEAV =  e.parameters.mEAV_val;
    mGAS =  e.parameters.mGAS_val;
    CRC2 =  e.parameters.CRC2_val;
    CRC3 =  e.parameters.CRC3_val;

    
    console.log (mEVLT); console.log (mEVHT);
  
    
    // save the data to spreadsheet
       save_data( Tbinnen, Tbuit, mEVLT, mEVHT,mEOLT, mEOHT, mEAT, mEAV, mGAS, CRC2, CRC3);
    
 
    return ContentService.createTextOutput("Wrote:\n  Tbinnen: " + Tbinnen + "\n  Tbuit: " + Tbuit);
 
  } catch(error) { 
    Logger.log(error);                      
    return ContentService.createTextOutput("oops...." + error.message 
                                            + "\n" + new Date() 
                                            + "\ntag: " + Tbinnen +
                                            + "\nvalue: " + Tbuit);
  }  
}
 

// Method to save given data to a sheet
function save_data(Tbinnen, Tbuit, mEVLT, mEVHT,mEOLT, mEOHT, mEAT, mEAV, mGAS ,CRC2, CRC3 ){
  Logger.log("--- save_data ---"); 

    

  try {
    var dateTime = new Date();
    var hrs = dateTime.getHours();
    var dag = dateTime.getDay();
    // Paste the URL of the Google Sheets starting from https thru /edit
    // For e.g.: https://docs.google.com/..../edit 
    var ss = SpreadsheetApp.openByUrl("https://docs.google.com/spreadsheets/d/17Rd1vln4Pz0dMnMCMzQ04Sd1nK82aKunuVM02VWW2m0/edit");
    //17Rd1vln4Pz0dMnMCMzQ04Sd1nK82aKunuVM02VWW2m0
    
    var summarySheet = ss.getSheetByName("Summary");
    var dataLoggerSheet = ss.getSheetByName("DataLogger");
    var longSheet = ss.getSheetByName("Long");
    var weekSortSheet = ss.getSheetByName("weekSort");
    var weekTimeSheet = ss.getSheetByName("weekTime");
    // Get last edited row from DataLogger sheet
    var row =     dataLoggerSheet.getLastRow() + 1;
    var rowlong = longSheet.getLastRow() + 1;
    var rowWeekSort = weekSortSheet.getLastRow()+1;
    var rowweekTime = weekTimeSheet.getLastRow();
    // populate longSheet met een waarde per hieronder aangegeven tijd per dag.
    if (summarySheet.getRange("B5").getValue()== 0){ // check already one value stored in longSheet
    if(hrs ==07 || hrs== 15 || hrs== 23 || hrs== 10) {  // hours to be stored
     // Start Populating the logdata sheet
    longSheet.getRange("A" + rowlong).setValue(rowlong -1); // ID
    longSheet.getRange("B" + rowlong).setValue(dateTime); // dateTime
    longSheet.getRange("D" + rowlong).setValue(parseFloat(Tbinnen)); // value
    longSheet.getRange("E" + rowlong).setValue(parseFloat(Tbuit)); // value
    longSheet.getRange("F" + rowlong).setValue(mEVLT); // value
    longSheet.getRange("G" + rowlong).setValue(mEVHT); // value
    longSheet.getRange("H" + rowlong).setValue(mEOLT); // value
    longSheet.getRange("I" + rowlong).setValue(mEOHT); // value
    longSheet.getRange("L" + rowlong).setValue(mGAS); // value
    summarySheet.getRange("B5").setValue(1); // indicate "semaphore"stored dwz gedaan 
  };
      
    };
    
   if(hrs !=07 && hrs!= 15 && hrs!= 23 && hrs!= 10) { 
    summarySheet.getRange("B5").setValue(0); // free storage of longSheet values
   }
    
    
    //Start populating int weeksortsheet
    var index = dag*24 + hrs+2; // bereken  plaats in de sheet
    
    weekSortSheet.getRange("B"+ index).setValue(dateTime);
    weekSortSheet.getRange("A"+ index).setValue(index);
    weekSortSheet.getRange("D"+ index).setValue(parseFloat(Tbinnen));
    weekSortSheet.getRange("E"+ index).setValue(parseFloat(Tbuit));
    weekSortSheet.getRange("F"+ index).setValue(mEVLT);
    weekSortSheet.getRange("G"+ index).setValue(mEVHT);
    weekSortSheet.getRange("H"+ index).setValue(mEOLT);
    weekSortSheet.getRange("I"+ index).setValue(mEOHT);
    weekSortSheet.getRange("J"+ index).setValue(mEAV);
    weekSortSheet.getRange("K"+ index).setValue(mEAT);
    weekSortSheet.getRange("L"+ index).setValue(mGAS);
        
    
    // Start Populating the DataLoggerSheet
    if (summarySheet.getRange("B6").getValue()== 1){ // check
    dataLoggerSheet.getRange("A" + row).setValue(row -1); // ID
    dataLoggerSheet.getRange("B" + row).setValue(dateTime); // dateTime
    dataLoggerSheet.getRange("D" + row).setValue(parseFloat(Tbinnen)); // value
    dataLoggerSheet.getRange("E" + row).setValue(parseFloat(Tbuit)); // value
    dataLoggerSheet.getRange("F" + row).setValue(mEVLT); // value
    dataLoggerSheet.getRange("G" + row).setValue(mEVHT); // value
    dataLoggerSheet.getRange("H" + row).setValue(mEOLT); // value
    dataLoggerSheet.getRange("I" + row).setValue(mEOHT); // value
    dataLoggerSheet.getRange("J" + row).setValue(mEAV); // value
    dataLoggerSheet.getRange("K" + row).setValue(mEAT); // value
    dataLoggerSheet.getRange("L" + row).setValue(mGAS); // value
    dataLoggerSheet.getRange("M" + row).setValue(CRC2); 
    dataLoggerSheet.getRange("N" + row).setValue(CRC3);
    }
    // Strat populating the weekTimeSheet
    if (weekTimeSheet.getRange("A2").getValue() != index) {
    weekTimeSheet.insertRows(2);
    weekTimeSheet.getRange("A" + 2).setValue(index); // ID
    weekTimeSheet.getRange("B" + 2).setValue(dateTime); // dateTime
    weekTimeSheet.getRange("D" + 2).setValue(parseFloat(Tbinnen)); // value
    weekTimeSheet.getRange("E" + 2).setValue(parseFloat(Tbuit)); // value
    weekTimeSheet.getRange("F" + 2).setValue(mEVLT); // value
    weekTimeSheet.getRange("G" + 2).setValue(mEVHT); // value
    weekTimeSheet.getRange("H" + 2).setValue(mEOLT); // value
    weekTimeSheet.getRange("I" + 2).setValue(mEOHT); // value
    weekTimeSheet.getRange("J" + 2).setValue(mEAV); // value
    weekTimeSheet.getRange("K" + 2).setValue(mEAT); // value
    weekTimeSheet.getRange("L" + 2).setValue(mGAS); // value
    weekTimeSheet.deleteRow(169);  //  7 dagen en 24 metingen per dag = 168 rows. Daarna deleten
  }
    // Update summary sheet
    summarySheet.getRange("B1").setValue(dateTime); // Last modified date
    summarySheet.getRange("B2").setValue(row - 1); // Count 
    summarySheet.getRange("B4").setValue(rowlong - 1); // Count 
    var recipient = Session.getActiveUser().getEmail();
    var subject = 'A list of files in your Google Drive';
    var body = Logger.getLog();
    MailApp.sendEmail(recipient, subject, body);
   
  }
 
  catch(error) {
    Logger.log(JSON.stringify(error));
  }
 
  Logger.log("--- save_data end---"); 
}
