import sys
import matplotlib.pyplot as plt
import numpy as np
import MySQLdb
import pickle


def getMarkovChainDenseMatrixWithFeature(Ti):
	#find P(l|x)
	Fxy=np.zeros((Ti.shape[0],Ti.shape[1],Ti.shape[2]))
	plx=np.zeros((Ti.shape[0],Ti.shape[2]))
	for fx in xrange(Ti.shape[0]):
	    lvec=np.zeros((1,Ti.shape[2]))
	    for fl in xrange(Ti.shape[2]):
	        lvec[0,fl]=np.sum(Ti[fx,:,fl])
	    lvec=(lvec>0).astype(np.float32)
	    lvec=lvec/np.sum(lvec)
	    plx[fx,:]=lvec

	#find P(x->y|l)
	T=Ti.astype(np.float32)
	T[T<1e-10]=1e-10
	for fr in xrange(T.shape[0]):
	    for fl in xrange(T.shape[2]):
	        row=T[fr,:,fl]
	        srow=np.sum(row)
	        T[fr,:,fl]=row/srow
	
	#get Mxy
	Mxy=np.zeros((T.shape[0],T.shape[1]))
	for fx in xrange(T.shape[0]):
	    print fx
	    for fy in xrange(T.shape[1]):
	        Fsum=np.zeros(T.shape[2])
	        for fl in xrange(T.shape[2]):
	            Fsum2=np.zeros(T.shape[2])
	            Fsum2[fl]=1.
	            Fsum+=plx[fx,fl]*T[fx,fy,fl]*Fsum2
	        Fxy[fx,fy,:]=Fsum
	        Mxy[fx,fy]=plx[fx,:].reshape((1,-1)).dot(T[fx,fy,:].reshape((-1,1)))
	
	#get Mlrw (lazy random walk process with random stop (lamda))
	lamda=0.5 #Stopping probability
	dmax=20
	Mlrw=np.zeros((T.shape[0],T.shape[1]))
	Flrw=np.transpose(np.zeros((T.shape[0],T.shape[1],T.shape[2])),(2,0,1))
	var=np.identity(T.shape[0])
	for f in xrange(dmax):
	    print f
	    var=var.dot(Mxy)*(1-lamda)
	    Mlrw+=var
	    Flrw+=Mlrw*np.transpose(Fxy,(2,0,1))
	Mlrw*=lamda
	Flrw=np.transpose(Flrw,(1,2,0))
	return T,plx,Mxy,Mlrw,Fxy,Flrw


def getMarkovChainDenseMatrix(Ti):
	#find P(l|x) - Uniform distribution of l given x
	plx=np.zeros((Ti.shape[0],Ti.shape[2]))
	for fx in xrange(Ti.shape[0]):
	    lvec=np.zeros((1,Ti.shape[2]))
	    for fl in xrange(Ti.shape[2]):
	        lvec[0,fl]=np.sum(Ti[fx,:,fl])
	    lvec=(lvec>0).astype(np.float32)
	    lvec=lvec/(np.sum(lvec)+1e-10)
	    plx[fx,:]=lvec

	#find P(x->y|l)
	T=Ti.astype(np.float32)
	T[T<1e-10]=1e-10
	for fr in xrange(T.shape[0]):
	    for fl in xrange(T.shape[2]):
	        row=T[fr,:,fl]
	        srow=np.sum(row)
	        T[fr,:,fl]=row/srow
	
	#get Mxy
	Mxy=np.zeros((T.shape[0],T.shape[1]))
	for fx in xrange(T.shape[0]):
	    for fy in xrange(T.shape[1]):
	        #nsum=0.0
	        #for fl in xrange(T.shape[2]):
	            #nsum+=plx[fx,fl]*T[fx,fy,fl]
	        #Mxy[fx,fy]=nsum
	        Mxy[fx,fy]=plx[fx,:].reshape((1,-1)).dot(T[fx,fy,:].reshape((-1,1)))
	
	#get Mlrw (lazy random walk process with random stop (lamda))
	lamda=0.5 #Stopping probability
	dmax=20
	Mlrw=np.zeros((T.shape[0],T.shape[1]))
	var=np.identity(T.shape[0])
	for f in xrange(dmax):
	    var=var.dot(Mxy)*(1-lamda)
	    Mlrw+=var
	Mlrw*=lamda
	
	return T,plx,Mxy,Mlrw
	
	        

def getMysqlTripleAsDenseMatrix():
	db = MySQLdb.connect(host="localhost",user="monty",passwd="some_pass", db="robot")        
	cur = db.cursor()
	cur.execute("SELECT distinct instance FROM FCSbookTripleInstListW")
	InstanceNames=list()
	for row in cur.fetchall():
		InstanceNames.append(row[0])
	cur.execute("SELECT * FROM FCSbookTripleRoleListW")
	RoleNames=list()
	for row in cur.fetchall():
		RoleNames.append(row[1])
	#T(subjectnum,objnum,propertynum)
	T=np.zeros((len(InstanceNames),len(InstanceNames),len(RoleNames)))
	T=T.astype(np.uint8)
	cur.execute("SELECT * FROM FCSbookTripleW")
	for row in cur.fetchall():
		subj=row[1]
		prop=row[2]
		obj=row[3]
		weight=row[4]
		#print subj,prop,obj
		for f in xrange(len(InstanceNames)):
			if(InstanceNames[f]==subj):
				Tsub=f
				break
		for f in xrange(len(InstanceNames)):
			if(InstanceNames[f]==obj):
				Tobj=f
				break
		for f in xrange(len(RoleNames)):
			if(RoleNames[f]==prop):
				Tprop=f
				break
		T[Tsub,Tobj,Tprop]=weight
		
	db.close()
	#T2 is T with inverted roles
	T1=np.transpose(T,(1,0,2))
	T2=np.concatenate((T,T1),axis=2)
	
	return T2,InstanceNames,RoleNames

def updateAndSaveMlrw():
#T,InstanceNames,RoleNames=getMysqlTripleAsDenseMatrix()
#_,_,Mxy,Mlrw=getMarkovChainDenseMatrix(T)
        T,InstanceNames,RoleNames=getMysqlTripleAsDenseMatrix()
        
        #find the types of the instances
        #ic:conceptContainer	
	#ic:questionContainer
	#ic:pictureContainer
	#ic:bookContainer
	#ic:termContainer
	#ic:explanationContainer
	InstanceTypeList=[]
	for f,fname in enumerate(InstanceNames):
	    if (fname[0:3]=="tc:"):
	    	InstanceTypeList.append("topic")
	    elif(fname[0:2]=="c:"):
	    	InstanceTypeList.append("ic:conceptContainer")
	    elif(fname[0:3]=="ic:"):
	    	InstanceTypeList.append("internalContainer")
	    elif(fname[0:8]=="o:descp:"):
	    	InstanceTypeList.append("ic:bookContainer")
	    elif(fname[0:2]=="q:"):
	    	InstanceTypeList.append("ic:questionContainer")
	    elif(fname[0:2]=="t:"):
	    	InstanceTypeList.append("ic:termContainer")
	    elif(fname[0:2]=="e:"):
	    	InstanceTypeList.append("ic:explanationContainer")
	    else:
	    	InstanceTypeList.append("unknown")
	
        
	_,_,Mxy,Mlrw=getMarkovChainDenseMatrix(T)
	with open('./models/object.pickle','w') as f:
	    pickle.dump([Mlrw,  InstanceNames, RoleNames, InstanceTypeList],f)

def loadMlrw():
	with open('./models/object.pickle') as f:
	    Mlrw,  InstanceNames, RoleNames,InstanceTypeList=pickle.load(f)
	return Mlrw,  InstanceNames, RoleNames,InstanceTypeList

def queryResult(containerType,queryNodes):
	#Example: queryResult("ic:questionContainer",["o:descp:c5arithmeticLogicUnit","tc:integer"])
	
	lenQueryNodes=len(queryNodes)
	Mlrw,  InstanceNames, RoleNames,InstanceTypeList=loadMlrw()
	Thres=0.1/Mlrw.shape[0]
	
	inputVec=np.zeros((1,Mlrw.shape[0]))
	
	for f in xrange(lenQueryNodes):
	    wordFoundFlag=0
	    for f2,fname in enumerate(InstanceNames):
	       if(fname==queryNodes[f]):
	          inputVec[0,f2]=1.0
	          wordFoundFlag=1
	          break
	    if(wordFoundFlag==0):
	        print "Query node not found"
	        print "##end##"
	        sys.exit()
	
	res=inputVec.dot(Mlrw)/lenQueryNodes
	
	
	rankList=np.array([[-1.,-1]])#instanceID,Score
	for f,fname in enumerate(InstanceTypeList):
		if(fname!=containerType or res[0,f]<Thres):
		    res[0,f]=0
		else:
		    rankList=np.append(rankList,np.array([[f,res[0,f]]]),axis=0)
	
	#rankList=np.sort(rankList,axis=0)[::-1]
	rankList=rankList[rankList[:,1].argsort()][::-1]
	
	if(rankList[0,0]==-1):
	   	print "None"
	else:
		for f in rankList:
		   if(f[0]!=-1):
		       print InstanceNames[int(f[0])],f[1] 

	
	return res,rankList
		         
	   

if __name__=="__main__":
   try:
      mode=int(sys.argv[1])
      print mode
      if(mode==1): #use, input:  container_type:node_IDs_comma_seperated 
           #Example: python graphWalkmysql.py 1 ic:questionContainer c:inputOutput
           print "##Start##"
      	   inp1=sys.argv[2] #container_type
      	   inp2=sys.argv[3] #query nodes
      	   inp2=inp2.split(",")
      	   res,rankList=queryResult(inp1,inp2)
      	   print "##End##"
      elif(mode==2): #update Markov Chain
           #Example: python graphWalkmysql.py 2 
      	   print "##Start##"
      	   updateAndSaveMlrw()
      	   print "##End##"
      
      
      
      
      
   except:
      print "Cannot find"

