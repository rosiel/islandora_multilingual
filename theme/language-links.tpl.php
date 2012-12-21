<?php 

/* This is a template file to process language-listing links. 
*  It takes in an array of arrays containing the keys 'label', 
*  'class', and 'value' and outputs them.
*/
?>

<div class="languages">
<?php if (empty($content)): ?>
  <p class="no-results"><?php print t('Nothing found.'); ?> </p>
<?php else: ?>
  <ul>
  <?php foreach ($content as $language => $details): ?>
    <li class="<?php print ($details['class']); ?>"><?php print ($details['value']); ?></li>
  <?php endforeach; ?>
  </ul>
<?php endif; ?>
  
</div>
