#ifndef PLAYTHATSHEET_H
#define PLAYTHATSHEET_H

#include <iostream>
#include <thread>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>
#include <functional>
#include <tinyxml2.h>

#define linearTimeConst 85    //how many ms to move 1 white key
#define defaultLinearVel 0xff   //default velocity

#define posPress 45
#define posRelease 0   
#define posReleaseBlack 20

#define posPedalPress 0x01
#define posPedalRelease 0x07

namespace SCORE {

    using namespace tinyxml2;
    using namespace std;

    int Tempo = 0;
    int Beats = 0;
    int BeatType = 0;
    int RPM = 255;
    int RPM_w = 255;
    int RPM_b = 255;

    /*static const bool isBlank[] = {
        true, true, true, false,
        true, true, true, true, true, false, true, true, true, true, true, true, true, false,
        true, true, true, true, true, false, true, true, true, true, true, true, true, false,
        true, true, true, true, true, false, true, true, true, true, true, true, true, false,
        true, true, true, true, true, false, true, true, true, true, true, true, true, false,
        true, true, true, true, true, false, true, true, true, true, true, true, true, false,
        true, true, true, true, true, false, true, true, true, true, true, true, true, false,
        true, true, true, true, true, false, true, true, true, true, true, true, true, false,
        true,
        };
    */

   static const bool isBlack[] = {
        false, true, false, false, true, false, true, false, false, true, false, true // G, G#, A, A#, B, C, C#, D, D#, E, F, F#
    };
   static const bool isBlank[]={
    true, true, true, false, true, true, true, true, true, false, true, true, true, true,
   };
    static const int KeyIndex[] = {
        1, 2, 3, 
    };

    struct blancKey{
        int firstblanc;
        int secondblanc;
    };

    blancKey blankIndex[12] = {
        {-1, -1},
        {4, 10},
        {-1, -1},
        {2, 8},
        {6, 14},
        {-1, -1},
        {4, 12},
        {-1, -1},
        {2, 10},
        {8, 14},
        {-1, -1},
        {6, 12},
    };

    long long t_PedalRelease = 0;
    int _isPedalPressed = 1;

    struct NOTE {
        int barNum;
        int noteNum;

        int octave;
        int key;
        float duration;
        float articulation_t;
        float articulation_p;
        int pedal = 0;
        int ID;
        int note_rpm;

        long long SP_finger=0;    //start point of finger
        long long SP_module=0;    //start point of module

        long long timeHold=0;    //time to hold
        long long timePress=0;    //time takes pressing key
        long long timeModule = 0;   //time takes moving module

        long long RP_finger=0;    //release point of inger
        long long RP_module=0;    //release point of module
        int moveOctave =0;
        int _isPressed = 0;
        int _isPlayed = 0;
        int _isWhite = 0;
    };

    int currentPosL = 0;
    int currentPosR = 0;

    int loadScoreFromXML(const std::string& filename, std::vector<NOTE>& notes) {
        int retVal = -1;
        XMLDocument doc;
        cout << "Loading Score from " << filename << endl;
        if (doc.LoadFile(filename.c_str()) != XML_SUCCESS) {
            cout << "Failed to load file" << endl;
            return retVal;
        }

        XMLElement* scorePartwise = doc.FirstChildElement("score-partwise");
        if (!scorePartwise) {
            cout << "<score-partwise> not found" << endl;
            return retVal;
        }
        std::cout << "score found" << std::endl;
        // Read <head> element
        XMLElement* head = scorePartwise->FirstChildElement("head");
        if (head) {
            XMLElement* tempo = head->FirstChildElement("tempo");
            Tempo = tempo->IntText();
            XMLElement* time = head->FirstChildElement("time");
            if (time) {
                XMLElement* beats = time->FirstChildElement("beats");
                Beats = beats->IntText();
                XMLElement* beatType = time->FirstChildElement("beat-type");
                BeatType = beatType->IntText();
            }
            XMLElement* defaultRpm_w = head->FirstChildElement("defaultRpm_w");
            RPM_w = defaultRpm_w->IntText();
            XMLElement* defaultRpm_b = head->FirstChildElement("defaultRpm_b");
            RPM_b = defaultRpm_b->IntText();
            
        }
        std::cout << "reading head done" << std::endl;
        // Read <part> element
        XMLElement* part = scorePartwise->FirstChildElement("part");
        if (!part) {
            cout << "<part> not found" << endl;
            return retVal;
        }
        std::cout << "reading part done" << std::endl;
        // Loop through each <BAR> element
        for (XMLElement* bar = part->FirstChildElement("BAR"); bar != nullptr; bar = bar->NextSiblingElement("BAR")) {
            int barNum = bar->IntAttribute("barNum");

            // Loop through each <NOTE> element within the <BAR>
            for (XMLElement* noteElement = bar->FirstChildElement("NOTE"); noteElement != nullptr; noteElement = noteElement->NextSiblingElement("NOTE")) {
                int noteNum = noteElement->IntAttribute("noteNum");
                std::cout << noteNum << std::endl;
                XMLElement* note = noteElement->FirstChildElement("note");
                if (note) {
                    NOTE noteData;
                    noteData.barNum = barNum;
                    noteData.noteNum = noteNum;
                    XMLElement* ID = note->FirstChildElement("ID");
                    if(ID) noteData.ID = ID->IntText();
                    XMLElement* pitch = note->FirstChildElement("pitch");
                    if (pitch) {
                        XMLElement* octave = pitch->FirstChildElement("octave");
                        XMLElement* key = pitch->FirstChildElement("key");
                        if (octave) noteData.octave = octave->IntText();
                        if (key) noteData.key = key->IntText();
                    }
                    XMLElement* duration = note->FirstChildElement("duration");
                    if (duration) noteData.duration = duration->FloatText();
                    XMLElement* articulation_t = note->FirstChildElement("articulation_t");
                    if (articulation_t) noteData.articulation_t = articulation_t->FloatText();
                    XMLElement* articulation_p = note->FirstChildElement("articulation_p");
                    if (articulation_p) noteData.articulation_p = articulation_p->FloatText();
                    XMLElement* pedal = note->FirstChildElement("pedal");
                    if (pedal) noteData.pedal = pedal->IntText();

                    notes.push_back(noteData);
                }
            }
        }
        
        retVal = 0; // Successfully loaded
        std::cout << "reading bar done" << std::endl;
        return retVal;
    }

   

    /// @brief 키 번호와 rpm을 입력받아 해당 모터를 작동시키는 키 누르기 함수
    /// @param motor : 디바이스 객체
    /// @param key : 키 번호
    /// @param rpm  : 회전 속도
    void pressKey(FingerMotor &motor, uint8_t key, uint8_t rpm) {
        if (key > motor.g_deviceQuantity || key < 0) {
            std::cerr << "Invalid Key" << std::endl;
            return;
        }

        uint8_t goalPosition = posPress;
        motor.setPosition(key, rpm, goalPosition);
    }

    /// @brief 키 번호와 rpm을 입력받아 해당 모터를 작동시키는 키 떼기 함수
    /// @param motor : 디바이스 객체
    /// @param key : 키 번호
    /// @param rpm : 회전 속도
    void releaseKey(FingerMotor &motor, uint8_t key, uint8_t rpm) {
        if (key > motor.g_deviceQuantity || key < 0) {
            std::cerr << "Invalid Key" << std::endl;
            return;
        }
        uint8_t goalPosition ;
        //MotorParameter param = motor.ReadMotorParameterByID(motor.paramFilePath, key);
        
        goalPosition = posRelease;
        motor.setPosition(key, rpm, goalPosition);
    }

    void releaseBlackKey(FingerMotor &motor, uint8_t key, uint8_t rpm) {
        if (key > motor.g_deviceQuantity || key < 0) {
            std::cerr << "Invalid Key" << std::endl;
            return;
        }
        uint8_t goalPosition ;
        //MotorParameter param = motor.ReadMotorParameterByID(motor.paramFilePath, key);
        
        goalPosition = posReleaseBlack;
        motor.setPosition(key, rpm, goalPosition);
    }

    /// @brief 페달을 누르는 함수
    /// @param motor 
    /// @param rpm 
    void pressPedal(FingerMotor &motor, uint8_t rpm) {
        motor.setPosition(3, rpm, posPedalPress);
    }

    /// @brief 페달을 떼는 함수
    /// @param motor 
    /// @param rpm 
    void releasePedal(FingerMotor &motor, uint8_t rpm) {
        motor.setPosition(3, rpm, posPedalRelease);
    }

    /// @brief 옥타브와 rpm을 입력받아 해당 모터를 옥타브로 이동시키는 함수
    /// @param motor 
    /// @param octave 
    /// @param rpm 
    /// @param ID 
    /// @return 움직이는 간격 계산결과값 반환(칸수)
    int moveToOctave(FingerMotor &motor, uint8_t octave, uint8_t rpm, uint8_t ID, int activeMotor) {
        uint8_t goalPosition = 0;
        int retVal = 0;
        if(ID == 1){
            std::cout << "Left Hand ";
        }
        else if(ID == 2){
            std::cout << "Right Hand ";
        }
        else{
            std::cerr << "Invalid ID" << std::endl;
            return retVal;
        }
        
        if(octave == 0 && ID == 1){
            std::cout << "Move to 0 Octave" << std::endl;
            if(activeMotor) motor.setPosition(ID, rpm, 0);
            retVal = currentPosL;  //how many steps moved
            currentPosL = 0;
            return retVal;
        }
        else if(octave >0 && octave < 7){
            std::cout << "Move to " << (int)octave << " Octave" << std::endl;
            if(ID == 1){    //case left hand
                goalPosition = 2 + (octave-1)*7;
                if(goalPosition + 7 > 51-(currentPosR+7)){
                    std::cerr << "Invalid Position Left" << std::endl;
                    return retVal;//case push
                }
                if(activeMotor) motor.setPosition(ID, rpm, -goalPosition);
                retVal = abs(currentPosL-goalPosition);  //how many steps moved
                currentPosL = goalPosition;
                return retVal;
            }
            else if (ID == 2){  //case right hand
                goalPosition = (7 - octave)*7;
                if(51-(goalPosition+7) < currentPosL+7){
                    std::cerr << "Invalid Position Right" << std::endl;
                    return retVal;//case push
                }
                if(activeMotor) motor.setPosition(ID, rpm, -goalPosition);
                retVal = abs(currentPosR-51+goalPosition);  //how many steps moved
                currentPosR = 51 - goalPosition;
                return retVal;
            }
            else{
                std::cerr << "Invalid ID" << std::endl;
                return retVal;
            }
        }
        else if(octave == 7 && ID == 2){
            std::cout << "Move to 7 Octave" << std::endl;
            if(activeMotor)motor.setPosition(ID, rpm, 0);
            retVal = 51 - currentPosR;  //how many steps moved
            currentPosL = 0;
            return retVal;
        }
        else{
            std::cerr << "Invalid Octave" << std::endl;
            return retVal;
        }
    }

    /// @brief 흰 건을 입력받아 해당 모터를 흰 건으로 이동시키는 함수
    /// @param motor 
    /// @param whiteKey 
    /// @param rpm 
    /// @param ID 
    /// @param activeMotor 
    /// @return 
    int moveToWhiteKey(FingerMotor &motor, uint8_t whiteKey, uint8_t rpm, uint8_t ID, int activeMotor) {
        uint8_t goalPosition = 0;
        int retVal = 0;
        if(ID == 1){
            std::cout << "Left Hand ";
        }
        else if(ID == 2){
            std::cout << "Right Hand ";
        }
        else{
            std::cerr << "Invalid ID" << std::endl;
            return retVal;
        }

        
        if(ID == 1){
            retVal = abs(currentPosL - whiteKey);  //how many steps moved
            std::cout << currentPosL << " -> " << (int)whiteKey << "th white ";
            if(activeMotor) motor.setPosition(ID, rpm, currentPosL-whiteKey);
            currentPosL = whiteKey;
            return retVal;
        }
        else if(ID == 2){
            retVal = abs(currentPosR - whiteKey);  //how many steps moved
            std::cout << currentPosR << " -> " << (int)whiteKey << "th white ";
            if(activeMotor) motor.setPosition(ID, rpm, currentPosR-whiteKey);
            currentPosR = whiteKey;
            return retVal;
        }
        else{
            std::cerr << "Invalid ID" << std::endl;
            return retVal;
        }

        
    }

    /// @brief db 테스트 함수
    /// @param motor
    void dbTest(FingerMotor &motor) {
        std::cout << "dB Test start" << std::endl;
        usleep(1000000);
        std::cout << "move to 4th octave" << std::endl;
        //int time4sleep = moveToOctave(motor, 4, defaultLinearVel, 1, 1);   //move left hand to 4th octave
        int time4sleep = moveToWhiteKey(motor, 23, defaultLinearVel, 1, 1);   //move left hand to 4th octave
        std::cout << "Sleep for "<< time4sleep*linearTimeConst << "ms" << std::endl;
        usleep(time4sleep*linearTimeConst*1000);
        std::cout << "ready for octave test" << std::endl;
    }


    


    /// @brief 88개의 키 중에서 주어진 키가 몇 번째 백건인지 계산하는 함수
    /// @param fullKey 
    /// @param direction 
    /// @return 
    int fullToWhiteKey(int fullKey, int direction) {
    // 각 키의 백건 여부 판별
        static const bool isWhiteKey[] = {
            true, false, true, true, false, true, false, true,  //  A, A#, B, C, C#, D, D#, E
            true, false, true, false,   // F, F#, G, G#
        };
        if(fullKey == 0){
            return -1;
        }
        // 1-based fullKey를 0-based 인덱스로 변환
        fullKey -= 1;

        // 백건 카운트
        int whiteCount = 0;

        // 전체 키를 순회하면서 백건 카운트
        for (int i = 0; i <= fullKey; i++) {
            if (isWhiteKey[i % 12]) whiteCount++;
        }

        // 현재 키가 백건이면 바로 반환
        if (isWhiteKey[fullKey % 12]) return whiteCount - 1;
        std::cout << "black ";
        // 흑건인 경우 direction에 따라 처리
        if (direction == 2) {  // 낮은음의 백건
            // 흑건 전에 나오는 마지막 백건을 찾기
            for (int i = fullKey - 1; i >= 0; i--) {
                if (isWhiteKey[i % 12]) {
                    return fullToWhiteKey(i + 1, direction);
                }
            }
        } else if (direction == 1) {  // 높은음의 백건
            // 흑건 후에 나오는 첫 번째 백건을 찾기
            for (int i = fullKey + 1; i < 88; i++) {
                if (isWhiteKey[i % 12]) {
                    return fullToWhiteKey(i + 1, direction);
                }
            }
        }

        // 적절한 백건을 찾지 못한 경우 (오류 방지)
        return -1;
    }

    /// @brief 백건을 입력받아 88키 매핑을 계산하는 함수
    /// @param whiteKey 
    /// @return 88keynumber
    int whiteToFullKey(int whiteKey){
        int octave = whiteKey / 7;
        int index = whiteKey % 7;
        static const int keyOffset[7] = {1, 3, 4, 6, 8, 9, 11};

        return octave * 12 + keyOffset[index];
    }


    void calKey(std::vector<SCORE::NOTE>& notes, FingerMotor &motor) {
        notes[0].SP_finger = 0;
        notes[0].SP_module = 0;
        int prevID1 = 0;
        int prevID2 = 0;
        int pprevID1 = 0;
        int pprevID2 = 0;
        int currModule1 = 0;
        int currModule2 = 0;

        for(size_t i=0; i<notes.size(); i++){
            if(notes[i].ID == 1) {
                prevID1 = i;
                break;
            }
        }
        std::cout << "Calculating Key " <<  notes.size() << std::endl;
        for(size_t i=0; i<notes.size(); i++){
            int RPM;
            if(isBlack[(notes[i].key - 1) % 12]) {
                std::cout<<i<<"black key " << notes[i].key;
                RPM = RPM_b;
                notes[i].note_rpm = RPM_b;
            }
            else {
                RPM = RPM_w;
                std::cout<<i<<"white key "<< notes[i].key;
                notes[i].note_rpm = RPM_w;
                notes[i]._isWhite = 1;
            }
            notes[i].timeHold = notes[i].duration * 60000 / Tempo;     //time to hold, mSec
            //notes[i].timeHold = notes[i].duration * 60000 / Tempo * notes[i].articulation_t; original

            //notes[i].timePress = (int)( 1000000 / 60 * 8/RPM) * notes[i].articulation_p;    //degree per mS no Articulation
            if(notes[i]._isWhite == 1){notes[i].timePress = (int)(60000 / (RPM * notes[i].articulation_p));}    //white key
            else{
                notes[i].timePress = (int)(25*60000 / (RPM*45 * notes[i].articulation_p));    //degree per mS
            }
            //notes[i].timePress = (int)(45*60000 / (RPM*45* notes[i].articulation_p));    //degree per mS
            notes[i].octave = SCORE::fullToWhiteKey(notes[i].key, notes[i].ID); 
            if(i>0) {
                
                if(notes[i].ID == 1 && i > prevID1){    //left hand
                    notes[i].SP_finger = notes[prevID1].timeHold + notes[prevID1].timePress +  notes[prevID1].SP_finger - notes[i].timePress;    //start point of finger
                    
                    if(notes[i].key == 0){ //rest
                        //notes[i].SP_module = notes[prevID1].timeHold + notes[prevID1].SP_module - notes[i].timePress;
                    }
                    else if(whiteToFullKey(currModule1) >  notes[i].key){   //left hand move octave to left (fullkey)
                        std::cout << whiteToFullKey(currModule1) << std::endl;
                        //currModule1 = notes[i].octave;  //pose in white key
                        currModule1 = notes[i].octave - 6;
                        notes[i].moveOctave = 1;
                        int time2sleep = moveToWhiteKey(motor,currModule1 , defaultLinearVel, 1, 0);   //move left hand 
                        notes[i].SP_module = notes[i].SP_finger - time2sleep * linearTimeConst;
                        notes[prevID1].timeHold = notes[prevID1].timeHold - time2sleep * linearTimeConst - notes[prevID1].timePress - notes[i].timePress;
                        if(notes[prevID1].key == notes[i].key){
                            notes[prevID1].timeHold = notes[prevID1].timeHold - notes[prevID1].timePress - notes[i].timePress;
                            std::cout << "same key "; 
                        }
                        std::cout << prevID1 << "'s new hold Time: " << notes[prevID1].timeHold << std::endl;
                    }
                    else if(whiteToFullKey(currModule1) + 11 < notes[i].key){   //left hand move octave to right (fullkey)
                        std::cout << whiteToFullKey(currModule1) << std::endl;
                        currModule1 = notes[i].octave - 6; //pose in white key
                        notes[i].moveOctave = 1;
                        int time2sleep = moveToWhiteKey(motor,currModule1 , defaultLinearVel, 1, 0);   //move left hand 
                        notes[i].SP_module = notes[i].SP_finger - time2sleep * linearTimeConst;
                        notes[prevID1].timeHold = notes[prevID1].timeHold - time2sleep * linearTimeConst - notes[prevID1].timePress - notes[i].timePress;
                        if(notes[prevID1].key == notes[i].key){
                            notes[prevID1].timeHold = notes[prevID1].timeHold - notes[prevID1].timePress - notes[i].timePress;
                            std::cout << "same key "; 
                        }
                        std::cout << prevID1 << "'s new hold Time: " << notes[prevID1].timeHold << std::endl;
                    }
                    if(notes[prevID1].key == notes[i].key){
                            notes[prevID1].timeHold = notes[prevID1].timeHold - notes[prevID1].timePress - notes[i].timePress;
                            std::cout << "same key "; 
                        }

                    if(notes[pprevID1].key == notes[i].key && notes[pprevID1].SP_finger + (1+notes[pprevID1].articulation_p)*notes[pprevID1].timePress + notes[pprevID1].timeHold > notes[i].SP_finger){
                        notes[pprevID1].timeHold = notes[pprevID1].timeHold - notes[i].timePress;
                        std::cout << "ssame key ";
                    }
                } 
                else if(notes[i].ID == 2 && i > prevID2){     //right hand
                    notes[i].SP_finger = notes[prevID2].timeHold + notes[prevID2].timePress + notes[prevID2].SP_finger - notes[i].timePress;
                    
                    if(notes[i].key == 0){ //rest
                        //notes[i].SP_module = notes[prevID2].timeHold + notes[prevID2].SP_module - notes[i].timePress;
                    }
                    else if(whiteToFullKey(44 - currModule2) >  notes[i].key){   //right hand move octave to left(fullkey)
                        currModule2 = 44 - notes[i].octave;//pose in white key
                        notes[i].moveOctave = 1;
                        int time2sleep = moveToWhiteKey(motor, currModule2, defaultLinearVel, 2, 0);   //move left hand
                        notes[i].SP_module = notes[i].SP_finger - time2sleep * linearTimeConst;
                        notes[prevID2].timeHold = notes[prevID2].timeHold - time2sleep * linearTimeConst - notes[prevID2].timePress - notes[i].timePress;
                        if(notes[prevID2].key == notes[i].key){
                            notes[prevID2].timeHold = notes[prevID2].timeHold - notes[prevID2].timePress - notes[i].timePress;
                            std::cout << "same key "; 
                        }
                        std::cout << prevID2 << "'s new hold Time: " << notes[prevID2].timeHold << std::endl;
                    }
                    else if(whiteToFullKey(44 - currModule2) + 12 < notes[i].key){   //right hand move octave to right(fullkey)
                        currModule2 = 44 - notes[i].octave;//pose in white key
                        notes[i].moveOctave = 1;
                        int time2sleep = moveToWhiteKey(motor, currModule2, defaultLinearVel, 2, 0);   //move left hand 
                        notes[i].SP_module = notes[i].SP_finger - time2sleep * linearTimeConst;
                        notes[prevID2].timeHold = notes[prevID2].timeHold - time2sleep * linearTimeConst - notes[prevID2].timePress - notes[i].timePress;
                        if(notes[prevID2].key == notes[i].key){
                            notes[prevID2].timeHold = notes[prevID2].timeHold - notes[prevID2].timePress - notes[i].timePress;
                            std::cout << "same key ";
                        }
                        std::cout << prevID2 << "'s new hold Time: " << notes[prevID2].timeHold << std::endl;
                    }
                    if(notes[prevID2].key == notes[i].key){
                            notes[prevID2].timeHold = notes[prevID2].timeHold - notes[prevID2].timePress - notes[i].timePress;
                            std::cout << "same key ";
                        }
                    if(notes[pprevID2].key == notes[i].key && notes[pprevID2].SP_finger + (1+notes[pprevID2].articulation_p)*notes[pprevID2].timePress + notes[pprevID2].timeHold > notes[i].SP_finger){
                        notes[pprevID2].timeHold = notes[pprevID2].timeHold - notes[i].timePress;
                        std::cout << "ssame key ";
                    }
                    if(notes[pprevID2].key == notes[i].key){
                        
                    }
                } 
            }

            if(notes[i].ID == 1) {
                pprevID1 = prevID1;
                prevID1 = i;
                
                }
            else if(notes[i].ID == 2) {
                pprevID2 = prevID2;
                prevID2 = i;
                
                }

            
        
            std::cout <<"ID : " << notes[i].ID <<  " Bar : " << notes[i].barNum << " Note : " << notes[i].noteNum << " key : " << notes[i].key  << " duration : " << notes[i].duration<< " octave : " << notes[i].octave  << " timeH : " << notes[i].timeHold << " spF : " << notes[i].SP_finger <<  " spM: " << notes[i].SP_module << " tP: "<< notes[i].timePress <<" mv: "<<notes[i].moveOctave  << " a_t: " << notes[i].articulation_t<<  std::endl;

        }
    }


    /// @brief 기준음이 주어졌을 때 해당 음을 누르는 함수
    /// @param lfinger : 왼손 모터 객체
    /// @param rfinger : 오른손 모터 객체
    /// @param ID : 손의 ID
    /// @param noteH : 누르는 음의 높이
    void pressNote(FingerMotor &lfinger, FingerMotor &rfinger, int ID, int noteH, int rpm) {
        if(noteH == 0){
            std::cout << "Rest" << std::endl;
            return;
        }
        
        if(ID == 1){
            std::cout << "lhand curr " << whiteToFullKey(currentPosL) <<" press " << noteH - whiteToFullKey(currentPosL)+1;
            std::cout << " index1: " << blankIndex[whiteToFullKey(currentPosL)%12].firstblanc << " ,2: " << blankIndex[whiteToFullKey(currentPosL)%12].secondblanc << std::endl;
            int keyindex = 0;
            if(blankIndex[whiteToFullKey(currentPosL)%12].firstblanc <= (noteH - whiteToFullKey(currentPosL)+1) 
            && blankIndex[whiteToFullKey(currentPosL)%12].secondblanc > (noteH - whiteToFullKey(currentPosL)+2)){
                keyindex = noteH - whiteToFullKey(currentPosL)+2;
            }
            else if(blankIndex[whiteToFullKey(currentPosL)%12].secondblanc <= (noteH - whiteToFullKey(currentPosL)+2)){
                keyindex = noteH - whiteToFullKey(currentPosL)+3;
            }
            else{
                keyindex = noteH - whiteToFullKey(currentPosL)+1;
            }
            std::cout << " keyindex " << keyindex << std::endl;
            pressKey(lfinger, keyindex, rpm);
        }
        else if(ID == 2){
            std::cout << "rhand curr "<< whiteToFullKey(44-currentPosR) <<" press "<< noteH - whiteToFullKey(44-currentPosR)+1;
            int keyindex = 0;
            if(blankIndex[whiteToFullKey(44-currentPosR)%12].firstblanc <= noteH - whiteToFullKey(44-currentPosR)+1 
            && blankIndex[whiteToFullKey(44-currentPosR)%12].secondblanc > noteH - whiteToFullKey(44-currentPosR)+2){
                keyindex = noteH - whiteToFullKey(44-currentPosR)+2;
            }
            else if(blankIndex[whiteToFullKey(44-currentPosR)%12].secondblanc <= noteH - whiteToFullKey(44-currentPosR)+2){
                keyindex = noteH - whiteToFullKey(44-currentPosR)+3;
            }
            else{
                keyindex = noteH - whiteToFullKey(44-currentPosR)+1;
            }
            std::cout << " keyindex " << keyindex << std::endl;
            pressKey(rfinger, keyindex, rpm);
        }
        else{
            std::cerr << "Invalid ID" << std::endl;
            return;
        }
    }

    /// @brief 기준음이 주어졌을 때 해당 음을 떼는 함수
    /// @param lfinger : 왼손 디바이스 객체
    /// @param rfinger : 오른손 디바이스 객체
    /// @param ID : 손의 ID
    /// @param noteH : 떼는 음의 높이
    void releaseNote(FingerMotor &lfinger, FingerMotor &rfinger, int ID, int noteH, int _RPM, int _isWhite) {
        if(noteH == 0){
            std::cout << "Rest" << std::endl;
            return;
        }
        
        if(ID == 1){
            int keyindex = 0;
            if(blankIndex[whiteToFullKey(currentPosL)%12].firstblanc <= noteH - whiteToFullKey(currentPosL)+1 
            && blankIndex[whiteToFullKey(currentPosL)%12].secondblanc > noteH - whiteToFullKey(currentPosL)+2){
                keyindex = noteH - whiteToFullKey(currentPosL)+2;
            }
            else if(blankIndex[whiteToFullKey(currentPosL)%12].secondblanc <= noteH - whiteToFullKey(currentPosL)+2){
                keyindex = noteH - whiteToFullKey(currentPosL)+3;
            }
            else{
                keyindex = noteH - whiteToFullKey(currentPosL)+1;
            }
            std::cout << " keyindex " << keyindex << std::endl;
            if(_isWhite == 1){
                releaseKey(lfinger, keyindex, _RPM);
            }
            else{
                releaseBlackKey(lfinger, keyindex, _RPM);
            }
            //releaseKey(lfinger, keyindex, _RPM);
        }
        else if(ID == 2){
            std::cout << "rhand curr "<< whiteToFullKey(44-currentPosR) <<" press "<< noteH - whiteToFullKey(44-currentPosR)+1;
            int keyindex = 0;
            if(blankIndex[whiteToFullKey(44-currentPosR)%12].firstblanc <= noteH - whiteToFullKey(44-currentPosR)+1 
            && blankIndex[whiteToFullKey(44-currentPosR)%12].secondblanc > noteH - whiteToFullKey(44-currentPosR)+2){
                keyindex = noteH - whiteToFullKey(44-currentPosR)+2;
            }
            else if(blankIndex[whiteToFullKey(44-currentPosR)%12].secondblanc <= noteH - whiteToFullKey(44-currentPosR)+2){
                keyindex = noteH - whiteToFullKey(44-currentPosR)+3;
            }
            else{
                keyindex = noteH - whiteToFullKey(44-currentPosR)+1;
            }
            std::cout << " keyindex " << keyindex << std::endl;
            if(_isWhite == 1){
                releaseKey(rfinger, keyindex, _RPM);
            }
            else{
                releaseBlackKey(rfinger, keyindex, _RPM);
            }
            //releaseKey(rfinger, keyindex, _RPM);
        }
        else{
            std::cerr << "Invalid ID" << std::endl;
            return;
        }
    }


}

#endif // !PLAYTHATSHEET_H
