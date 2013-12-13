//
//  muza-personalit.cpp
//  Muza
//
//  Created by David Grunzweig on 12/4/13.
//
//
//basic c stuff
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <ApplicationServices/ApplicationServices.h>

//files
#include "raka-audio.h"
#include "muza-personality.h"


struct responseSet {
    double index;
    std::string response;
    int moodIndex;
    responseSet(double a, std::string b, int c){
        index = a;
        response = b;
        moodIndex = c;
    }
    double getIndex(){
        return index;
    }
    std::string getResponse(){
        return response;
    }
    int getMoodIndex(){
        return moodIndex;
    }
};

struct pronounSet {
    std::string pronoun;
    int index;
    pronounSet(std::string a, int b){
        pronoun = a;
        index = b;
    }
    std::string getPronoun(){
        return pronoun;
    }
    int getIndex(){
        return index;
    }
};

struct subjectsSet {
    std::string subject;
    int index;
    subjectsSet(std::string a, int b){
        subject = a;
        index = b;
    }
    std::string getSubject(){
        return subject;
    }
    int getIndex(){
        return index;
    }
};

struct moodSet {
    int index;
    std::string mood;
    moodSet(int a, std::string b){
        index = a;
        mood = b;
    }
    std::string getMood(){
        return mood;
    }
    int getIndex(){
        return index;
    }
};

struct moodyResponse {
    double index;
    std::string response;
    moodyResponse(double a, std::string b){
        index = a;
        response = b;
    }
    double getIndex(){
        return index;
    }
    std::string getResponse(){
        return response;
    }
};

//global variables
std::vector<responseSet> responses;
std::vector<std::string> defaults;
std::vector<pronounSet> pronouns;
std::vector<subjectsSet> subjects;
std::vector<moodSet> moods;
std::vector<moodyResponse> moodyResponses;
std::vector<int> previousSubjectIndexes;

//VoiceSpec * MuzaVoice;
//SpeechChannel chan;

double moodinessFactor = .1;
int moodIndex = 2;
std::string mood = "Calm";
int matches = 0;
int responseCount = 0;
int rootPitch = 60;

/**
 * Initializes an array of subjects
 */
bool initializeSubjects() {
    std::ifstream subjectsFile("subjects.txt");
    int index;
    std::string subject;
    std::string line;
    try {
        while(!subjectsFile.eof()){
            std::getline(subjectsFile, line);
            index = atoi(line.c_str());
            std::getline(subjectsFile, subject);
            subjectsSet set = *new subjectsSet(subject, index);
            subjects.push_back(set);
        }
        return true;
    } catch(std::exception& e){
        std::cout<<e.what()<<std::endl;
        
    }
    return false;
}
/**
 * Initializes an array of default responses.
 * @param args a string array containing the commandline arguments
 */
bool initializeDefaults(){
    std::ifstream defaultsFile ("defaults.txt");
    std::string line;
    try {
        while(!defaultsFile.eof()){
            std::getline(defaultsFile, line);
            defaults.push_back(line);
        }
        return true;
    } catch(std::exception& e){
        std::cout<<e.what()<<std::endl;
        return false;
    }
}
/**
 * Initializes an array of pronouns.
 * @param args a string array containing the commandline arguments
 */

bool initializePronouns(){
    std::ifstream pronounsFile ("pronouns.txt");
    int index;
    std::string line;
    std::string line2;
    try {
        while(!pronounsFile.eof()){
            std::getline(pronounsFile, line);
            std::getline(pronounsFile, line2);
            index = atoi(line2.c_str());
            pronounSet set = *new pronounSet(line, index);
            pronouns.push_back(set);
        }
        return true;
    } catch(std::exception& e){
        std::cout<<e.what()<<std::endl;
        return false;
    }
}

/**
 * Initializes an array of responses.
 * @param args a string array containing the commandline arguments
 */

bool initializeResponses(){
    std::ifstream responsesFile ("responses.txt");
    double index;
    int moodIndex;
    std::string response;
    std::string line;
    try {
        while(!responsesFile.eof()){
            std::getline(responsesFile, line);
            index = atof(line.c_str());
            std::getline(responsesFile, response);
            std::getline(responsesFile, line);
            moodIndex = atoi(line.c_str());
            responseSet set = *new responseSet(index, response, moodIndex);
            responses.push_back(set);
        }
        return true;
    } catch(std::exception& e){
        std::cout<<e.what()<<std::endl;
        return false;
    }
}

/**
 * Initializes an array of mood deltas.
 * @param args a string array containing the commandline arguments
 */

bool initializeMoods(){
    std::ifstream responsesFile ("moods.txt");
    int index;
    std::string mood;
    std::string line;
    try {
        while(!responsesFile.eof()){
            std::getline(responsesFile, line);
            index = atoi(line.c_str());
            std::getline(responsesFile, line);
            mood = line;
            moodSet set = *new moodSet(index, mood);
            moods.push_back(set);
        }
        return true;
    } catch(std::exception& e){
        std::cout<<e.what()<<std::endl;
        return false;
    }
}

bool initializeMoodyResponses(){
    std::ifstream responsesFile ("moodyResponses.txt");
    double index;
    std::string response;
    std::string line;
    try {
        while(!responsesFile.eof()){
            std::getline(responsesFile, line);
            index = atof(line.c_str());
            std::getline(responsesFile, line);
            response = line;
            moodyResponse set = *new moodyResponse(index, response);
            moodyResponses.push_back(set);
        }
        return true;
    } catch(std::exception& e){
        std::cout<<e.what()<<std::endl;
        return false;
    }
}
/**
 * reads the user response and searches the string for any of the terms in the terms array.
 * If a match of terms is found, prints the corresponding prompt for that term.
 * @param response a string that contains the users input
 */
vector<std::string> getWords(std::string response){
    vector<std::string> words;
    while (response.length() > 0){
        int wordEnd = (int)response.find_first_of(' ');
        if (wordEnd == std::string::npos){
            wordEnd = (int)response.length();
        }
        std::string word = response.substr(0, wordEnd);
        response.erase(0, wordEnd + 1);
        words.push_back(word);
    }
    return words;
    
}
std::string getResponse(std::string response){
    string outputText;
    bool pronounFound = false;
    bool subjectFound = false;
    int pronounIndex;
    int subjectIndex;
    
    vector<std::string>words = getWords(response);
    for (int j = 0; j < words.size(); j++){
        for (int i = 0; i < pronouns.size(); i++ ){
            if (words.at(j).compare(pronouns.at(i).getPronoun()) == 0 && !pronounFound){
                pronounIndex = pronouns.at(i).getIndex();
                pronounFound = true;
                break;
            }
        }
        for (int i = 0; i < subjects.size(); i++ ){
            if (words.at(j).compare(subjects.at(i).getSubject())  == 0 && !subjectFound ){
                subjectIndex = subjects.at(i).getIndex();
                subjectFound = true;
                previousSubjectIndexes.push_back(subjectIndex);
                break;
            }
        }
    }
    
    if (pronounFound && subjectFound){
        double responseIndex = pronounIndex + (double)subjectIndex/100;
        for (int i = 0; i < responses.size(); i++){
            if (responses.at(i).getIndex() == responseIndex){
                outputText.append("MUZA: " + responses.at(i).getResponse());
                moodIndex = responses.at(i).getMoodIndex();
                mood = moods.at(moodIndex-1).getMood();
                float pitch = rootPitch;// + (i/responses.size())*48;
                float velocity = .75;//+(i/responses.size());
                raka_playNotes( pitch, velocity, moodIndex );
                //SpeakThisText( responses.at(i).getResponse() );
                break;
            }
        }
        
//    } else if (pronounFound && !subjectFound){
//        double responseIndex = pronounIndex;
//        for (int i = 0; i < responses.size(); i++){
//            if (responses.at(i).getIndex() == responseIndex){
//                outputText.append("MUZA: " + responses.at(i).getResponse());
//                moodIndex = responses.at(i).getMoodIndex();
//                mood = moods.at(moodIndex-1).getMood();
//                float pitch = rootPitch;// + (i/responses.size())*48;
//                float velocity = .75;//+(i/responses.size());
//                raka_playNotes( pitch, velocity, moodIndex );
//                //SpeakThisText( responses.at(i).getResponse() );
//                break;
//            }
//        }
    } else if (!pronounFound && subjectFound){
        double responseIndex = 0 + (double)subjectIndex/100;
        for (int i = 0; i < responses.size(); i++){
            if (responses.at(i).getIndex() == responseIndex){
                outputText.append("MUZA: " + responses.at(i).getResponse());
                moodIndex = responses.at(i).getMoodIndex();
                mood = moods.at(moodIndex-1).getMood();
                float pitch = rootPitch;// + (i/responses.size())*48;
                float velocity = .75;//+(i/responses.size());
                raka_playNotes( pitch, velocity, moodIndex );
                //SpeakThisText( responses.at(i).getResponse() );
                break;
            }
        }
        
    }
    else {
        if ((double)rand()/RAND_MAX < moodinessFactor || previousSubjectIndexes.size() == 0){
            double index = moodIndex + (double)(rand() % 6)/10.0;
            for (int i = 0; i < moodyResponses.size(); i++){
                if (moodyResponses.at(i).getIndex() == index){
                    outputText.append("MUZA: " + moodyResponses.at(i).getResponse());
                    float pitch = rootPitch;// + (i/responses.size())*48;
                    float velocity = .75;//+(i/responses.size());
                    raka_playNotes( pitch, velocity, moodIndex );
                    //SpeakThisText( responses.at(i).getResponse() );
                    return outputText;
                }
            }
        } else {
            subjectIndex = previousSubjectIndexes.at(rand()%previousSubjectIndexes.size());
            double responseIndex = 0 + (double)subjectIndex/100;
            for (int i = 0; i < responses.size(); i++){
                if (responses.at(i).getIndex() == responseIndex){
                    outputText.append("MUZA: " + responses.at(i).getResponse());
                    moodIndex = responses.at(i).getMoodIndex();
                    mood = moods.at(moodIndex-1).getMood();
                    float pitch = rootPitch;// + (i/responses.size())*48;
                    float velocity = .75;//+(i/responses.size());
                    raka_playNotes( pitch, velocity, moodIndex );
                    //SpeakThisText( responses.at(i).getResponse() );
                    break;
                }
            }
        }
        //SpeakThisText( defaults.at(randomResponse) );
    }
    switch (moodIndex) {
        case 1:
            moodinessFactor = .5;
            break;
        case 2:
            moodinessFactor = .55;
            break;
        case 3:
            moodinessFactor = .65;
            break;
        case 4:
            moodinessFactor = .75;
            break;
        case 5:
            moodinessFactor = .85;
            break;
        case 6:
            moodinessFactor = .85;
            break;
    }
    return outputText;
}

std::string getMood(){
    return mood;
}

int getMoodIndex(){
    return moodIndex;
}

bool muzaInit(){
    if ( !initializeDefaults() || !initializePronouns() || !initializeResponses() || !initializeSubjects() || !initializeMoods() || !initializeMoodyResponses()){
        return false;
    }
    
    return true;
}


void impatience(){
    moodIndex = 6;
    mood = moods.at(moodIndex-1).getMood();
    play_muzaSong();
}

std::string welcomeMessage(){
    //print out the initial message
    std::string welcometext = "Hello, I am muza. How are you?";
    //SpeakThisText(welcometext);
    return welcometext;
}
