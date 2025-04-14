/**
 * @author cxx2020@outlook.com
 * @date 10:24 PM
*/

#pragma once

#include "CoreMinimal.h"

#define S_HTML_CONTENT(DESC, TITLE, BODY) \
	FString::Printf(TEXT("\
	<!DOCTYPE html> \
	<html lang=\"en\"> \
		<head> \
			<meta charset=\"utf-8\" /> \
			<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\" /> \
			<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\" /> \
			<meta name=\"description\" content=\"%s\" /> \
			<meta name=\"author\" content=\"cxx2020@outlook.com\" /> \
			<title>%s</title> \
			<link href=\"/css/styles.css\" rel=\"stylesheet\" /> \
			<link rel=\"icon\" href=\"/assets/img/logo.png\" /> \
		</head> \
		<body> \
			%s \
		</body> \
	</html>"), *DESC, *TITLE, *BODY)

#define S_HTML_401_BODY(COPYRIGHT) \
	FString::Printf(TEXT("\
	<div id=\"layoutError\"> \
        <div id=\"layoutError_content\"> \
            <main> \
                <div class=\"container\"> \
                    <div class=\"row justify-content-center\"> \
                        <div class=\"col-lg-6\"> \
                            <div class=\"text-center mt-4\"> \
                                <h1 class=\"display-1\">401</h1> \
                                <p class=\"lead\">Unauthorized</p> \
                                <p>访问这个资源没有权限.</p> \
                                <a href=\"/main/person\"> \
                                    <i class=\"fas fa-arrow-left me-1\"></i> \
                                    回到管理页 \
                                </a> \
                            </div> \
                        </div> \
                    </div> \
                </div> \
            </main> \
        </div> \
        <div id=\"layoutError_footer\"> \
            <footer class=\"py-4 bg-light mt-auto\"> \
                <div class=\"container-fluid px-4\"> \
                    <div class=\"d-flex align-items-center justify-content-between small\"> \
                        <div class=\"text-muted\">Copyright &copy; Your Website 2023</div> \
                        <div> \
                            <a href=\"#\">Privacy Policy</a> \
                            &middot; \
                            <a href=\"#\">Terms &amp; Conditions</a> \
                        </div> \
                    </div> \
                </div> \
            </footer> \
        </div> \
	</div>"), *COPYRIGHT)
	


#define S_HTML_404_BODY(COPYRIGHT) \
	FString::Printf(TEXT("\
	<div id=\"layoutError\"> \
		<div id=\"layoutError_content\"> \
			<main> \
				<div class=\"container\"> \
					<div class=\"row justify-content-center\"> \
						<div class=\"col-lg-6\"> \
							<div class=\"text-center mt-4\"> \
								<img class=\"mb-4 img-error\" src=\"/assets/img/error-404-monochrome.svg\" /> \
								<p class=\"lead\">在此服务器上找不到此请求的URL.</p> \
								<a href=\"/main/person\"> \
									<i class=\"fas fa-arrow-left me-1\"></i> \
									回到管理页 \
								</a> \
							</div> \
						</div> \
					</div> \
				</div> \
			</main> \
		</div> \
		<div id=\"layoutError_footer\"> \
			<footer class=\"py-4 bg-light mt-auto\"> \
				<div class=\"container-fluid px-4\"> \
					<div class=\"d-flex align-items-center justify-content-between small\"> \
						<div class=\"text-muted\">Copyright &copy; %s</div> \
						<div> \
							<a href=\"#\">Privacy Policy</a> \
							&middot; \
							<a href=\"#\">Terms &amp; Conditions</a> \
						</div> \
					</div> \
				</div> \
			</footer> \
		</div> \
	</div>"), *COPYRIGHT)


#define S_HTML_500_BODY(COPYRIGHT) \
	FString::Printf(TEXT("\
	<div id=\"layoutError\"> \
    	<div id=\"layoutError_content\"> \
    	    <main> \
    	        <div class=\"container\"> \
    	            <div class=\"row justify-content-center\"> \
    	                <div class=\"col-lg-6\"> \
    	                    <div class=\"text-center mt-4\"> \
    	                        <h1 class=\"display-1\">500</h1> \
    	                        <p class=\"lead\">内部服务器错误</p> \
    	                        <a href=\"/main/person\"> \
    	                            <i class=\"fas fa-arrow-left me-1\"></i> \
    	                            回到管理页面 \
    	                        </a> \
    	                    </div> \
    	                </div> \
    	            </div> \
    	        </div> \
    	    </main> \
    	</div> \
    	<div id=\"layoutError_footer\"> \
    	    <footer class=\"py-4 bg-light mt-auto\"> \
    	        <div class=\"container-fluid px-4\"> \
    	            <div class=\"d-flex align-items-center justify-content-between small\"> \
    	                <div class=\"text-muted\">Copyright &copy; %s</div> \
    	                <div> \
    	                    <a href=\"#\">Privacy Policy</a> \
    	                    &middot; \
    	                    <a href=\"#\">Terms &amp; Conditions</a> \
    	                </div> \
    	            </div> \
    	        </div> \
    	    </footer> \
    	</div> \
	</div>"), *COPYRIGHT)