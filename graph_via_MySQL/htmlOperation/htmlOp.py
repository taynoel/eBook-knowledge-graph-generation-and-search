from sklearn.feature_extraction.text import TfidfVectorizer
from HTMLParser import HTMLParser
import pickle
import gensim
import MySQLdb
import sys

# create a subclass and override the handler methods
class MyHTMLParser(HTMLParser):
    def handle_starttag(self, tag, attrs):
        print "Start tag:", tag
        for attr in attrs:
            print "     attr:", attr
    def handle_endtag(self, tag):
        print "Encountered an end tag :", tag

    def handle_data(self, data):
        print "Encountered some data  :", data

class MyHTMLParser2(HTMLParser):
    htmlList=[]
    #def __init__(self):
    #    self.htmlList=[]
    def handle_starttag(self, tag, attrs):
        #self.htmlList.append(["startTag",tag])
        tempList=["startTag",tag]
        for attr in attrs:
            for f in xrange(len(attr)):
              tempList.append(attr[f])
        self.htmlList.append(tempList)
    def handle_endtag(self, tag):
    	self.htmlList.append(["endTag",tag])

    def handle_data(self, data):
    	self.htmlList.append(["data",data])
       


def readFile(filename):
	with open(filename,'r')as myfile:
		data=myfile.read().replace("\n","")
	return data

def convertHtmlListToDescriptionList(htmlList):
	descpList=[]
	descpId=[]
	descpStr=""
	readMode=0
	for f,val in enumerate(htmlList): 
	    #Note: this approach is rigid...should me modified later
	    if readMode==0:
	       if(val[0]=="startTag" and val[1]=="p" and val[2]=="data-ontology" and val[3]=="true"):
	           #start reading data
	           readMode=1
	           descpId=val[5]
	           descpStr=""
	    elif readMode==1:
	       if(val[0]=="data"):
	          descpStr=descpStr+val[1]+" "
	       elif(val[0]=="endTag" and val[1]=="p"):
	          readMode=0
	          descpList.append([descpId,descpStr])
	return descpList
	       
	        
	        

def getListFromHtmlFile(filename):
	htmlDat=readFile(filename)
	parser = MyHTMLParser2()
	
	parser.feed(htmlDat)
	return parser.htmlList
	
def generateTermTriplefromHtmlFiles():
	db = MySQLdb.connect(host="localhost",user="monty",passwd="some_pass", db="robot")        
	cur = db.cursor()
	
	parser = MyHTMLParser2()
	parser.htmlList=[]
   	parser.feed(readFile("./html/dummyHTML.html"))
   	
   	descpList=convertHtmlListToDescriptionList(parser.htmlList)
   	
   	with open('./models/word2tfidf.pickle') as f:
		word2tfidf=pickle.load(f)
	word2tfidf=word2tfidf[0] #due to issue when saving
   	
   	descpWordList=[]
   	TotalPresentWords=[]
   	for f in xrange(len(descpList)):
   	    if (f%1000)==0:
   	        print f,"/",len(descpList)
   	    descpId=descpList[f][0]
   	    descpStr=descpList[f][1]
   	    presentWords=[]
   	    stnc_Token=list(gensim.utils.tokenize(descpStr, deacc=True, lower=True))
   	    for f2 in stnc_Token:
   	       if not(f2 in presentWords):
   	          if f2 in word2tfidf:
   	             if word2tfidf[f2]>3:
   	               try:
   	                  f2str=str(f2)
   	                  presentWords.append(f2str)
   	                  if not(f2 in TotalPresentWords):
   	                      TotalPresentWords.append(f2str)
   	               except:
   	                  print "Cannot decode",f2
   	    descpWordList.append([descpId,presentWords])
   	
   	
   	fi=open("./models/FoundationsofComputerScience_DicTermsInFile.txt","w")
   	for f in descpWordList:
   	    descpId=f[0]
   	    allWords=f[1]
   	    for f2 in allWords:
   	      try: 
   	        fi.write("nt:dicTermFor,te:"+str(f2)+","+str(descpId)+";\n")
   	        cur.execute("INSERT INTO FCSbookTripleW (subject,property,object,weight) VALUES (%s,%s,%s,%s)",("te:"+str(f2),"nt:dicTermFor",str(descpId),str(1.0)))
   	      except:
   	        print "error:",descpId,f2
   	
   	cur.execute("INSERT INTO FCSbookTripleRoleListW (role) VALUES ('nt:dicTermFor')")
   	for f in TotalPresentWords:
   	    strr="INSERT INTO FCSbookTripleInstListW (instance) VALUES ('te:"+f+"')"
   	    cur.execute(strr)
   	
   	fi.close()
   	db.commit()
   	db.close()
   	
   	return descpWordList,descpList,TotalPresentWords


if __name__=="__main__":
   try:
      mode=int(sys.argv[1])
      print mode
      if(mode==1): #add dictionary terms to mysql
           #Example: python htmlOp.py 1 
           print "##Start##"
      	   A,B,C=generateTermTriplefromHtmlFiles()
      	   print "##End##"
     

   except:
      print "Wrong argument"
# instantiate the parser and fed it some HTML
#parser = MyHTMLParser()
#parser.feed('<html><head><title>Test</title></head>'
            #'<body><h1>Parse <p>ooh</p>me!</h1></body></html>')
