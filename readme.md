To convert the AWS Certificates into der format  Use the following 
First install open ssl 

openssl x509 -in  da3b074ae4-certificate.pem.crt   -out cert.der -outform DER
openssl x509 -outform der -in AmazonRootCA1.pem -out ca.der
openssl rsa -in da3b074ae4-private.pem.key  -out priv.der -outform DER
