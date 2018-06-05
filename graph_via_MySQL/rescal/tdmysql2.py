#tensor decomposition, data from mysql
import logging
from scipy.io.matlab import loadmat
from scipy.sparse import lil_matrix
import rescal
import matplotlib.pyplot as plt
import numpy as np
import MySQLdb

logging.basicConfig(level=logging.INFO)

db = MySQLdb.connect(host="localhost",user="monty",passwd="some_pass", db="robot")        
cur = db.cursor()

cur.execute("SELECT * FROM FCSbookTripleInstListW")
InstanceNames=list()
for row in cur.fetchall():
	InstanceNames.append(row[1])
	
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
	T[Tsub,Tobj,Tprop]=1
			
db.close()

#RESCAL
T=T.astype(float)
X = [lil_matrix(T[:, :, k]) for k in range(T.shape[2])]

# Decompose tensor using RESCAL-ALS
A, R, fit, itr, exectimes = rescal.als(X, 500, init='nvecs', lambda_A=1, lambda_R=1)


res={}
for f,name in enumerate(RoleNames):
   res[name]=A.dot(R[f]).dot(A.T)

def NTshowNewLinks(linkThreshold):
  numEntities=T.shape[0]
  for f,name in enumerate(RoleNames):
     for fh in xrange(numEntities):
        for fw in xrange(numEntities):
           if(res[name][fh,fw]>=linkThreshold):
              if(T[fh,fw,f]==0):
                 print InstanceNames[fh]," ",RoleNames[f]," ",InstanceNames[fw]
   



