from sys import path
from sys import argv
path.append('/opt/anaconda3/envs/graphics/lib/python3.10.14/site-packages')
image_path = argv[1]

import cv2
import numpy as np
import glob

def perform_template_matching(input_image_path:str):
   # source = cv2.imread("../../../data/SAMPLES/P_PENT.png", cv2.IMREAD_GRAYSCALE)
   source = cv2.imread(input_image_path, cv2.IMREAD_GRAYSCALE)
   if source is None:
      # print("Could not load source image")
      return
    
   template_files = glob.glob("../../../data/SAMPLES/*.png")
   best_match = None
   best_match_value = 0.0
    
   for temp_file in template_files:
      templ = cv2.imread(temp_file, cv2.IMREAD_GRAYSCALE)
      if templ is None:
         # print(f"Could not load template: {temp_file}")
         continue
        
      result = cv2.matchTemplate(source, templ, cv2.TM_CCOEFF_NORMED)
      min_val, max_val, min_loc, max_loc = cv2.minMaxLoc(result)
        
      if max_val > best_match_value:
         best_match_value = max_val
         best_match = temp_file[22:-4]
    
   if best_match:
      # print(f"Best matching template: {best_match}")
      return best_match
   else:
      # print("No good match found")
      return None

if __name__ == "__main__":
   print(perform_template_matching(image_path))